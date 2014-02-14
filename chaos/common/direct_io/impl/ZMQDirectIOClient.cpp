/*
 *	ZMQDirectIOClinet.cpp
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2012 INFN, National Institute of Nuclear Physics
 *
 *    	Licensed under the Apache License, Version 2.0 (the "License");
 *    	you may not use this file except in compliance with the License.
 *    	You may obtain a copy of the License at
 *
 *    	http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
#include <chaos/common/utility/UUIDUtil.h>
#include <chaos/common/direct_io/impl/ZMQDirectIOClient.h>

#include <boost/format.hpp>

#include <zmq.h>
#include <string.h>
#include <assert.h>     /* assert */

#define ZMQDIO_LOG_HEAD "["<<getName()<<"] - "

#define ZMQDIOLAPP_ LAPP_ << ZMQDIO_LOG_HEAD
#define ZMQDIOLDBG_ LDBG_ << ZMQDIO_LOG_HEAD << __FUNCTION__ << " - "
#define ZMQDIOLERR_ LERR_ << ZMQDIO_LOG_HEAD

using namespace chaos::common::direct_io::impl;

typedef boost::unique_lock<boost::shared_mutex>	ZMQDirectIOClientWriteLock;
typedef boost::shared_lock<boost::shared_mutex> ZMQDirectIOClientReadLock;

void ZMQDirectIOClientFree (void *data, void *hint) {
    free (data);
}

static int read_msg(void* s, zmq_event_t* event, char* ep)
{
    int rc ;
    zmq_msg_t msg1;  // binary part
    zmq_msg_init (&msg1);
    zmq_msg_t msg2;  //  address part
    zmq_msg_init (&msg2);
    rc = zmq_msg_recv (&msg1, s, 0);
    if (rc == -1 && zmq_errno() == ETERM)
        return 1 ;
    rc = zmq_msg_recv (&msg2, s, 0);
    if (rc == -1 && zmq_errno() == ETERM)
        return 1;
    // copy binary data to event struct
    const char* data = (char*)zmq_msg_data(&msg1);
    memcpy(&(event->event), data, sizeof(event->event));
    memcpy(&(event->value), data+sizeof(event->event), sizeof(event->value));
    // copy address part
    const size_t len = zmq_msg_size(&msg2) ;
    ep = (char*)std::memcpy(ep, zmq_msg_data(&msg2), len);
    *(ep + len) = 0 ;
    return 0 ;
}

ZMQDirectIOClient::ZMQDirectIOClient(string alias):DirectIOClient(alias){
	priority_port = 0;
	service_port = 0;
	
	zmq_context = NULL;
	socket_priority = NULL;
	socket_service = NULL;
};

ZMQDirectIOClient::~ZMQDirectIOClient(){
};

//! Initialize instance
void ZMQDirectIOClient::init(void *init_data) throw(chaos::CException) {
    ZMQDIOLAPP_ << "Allocating zmq context";
    std::string priority_identity = UUIDUtil::generateUUIDLite();
    std::string service_identity = UUIDUtil::generateUUIDLite();
	int err = 0;
	int output_buffer_dim = 1;
    zmq_context = zmq_ctx_new();
    if(zmq_context == NULL) throw chaos::CException(0, "Error creating zmq context", __FUNCTION__);
    
    ZMQDIOLAPP_ << "Set number of thread for the contex";
    zmq_ctx_set(zmq_context, ZMQ_IO_THREADS, 2);
    
    ZMQDIOLAPP_ << "Allocating priority socket";
    socket_priority = zmq_socket (zmq_context, ZMQ_DEALER);
    if(socket_priority == NULL) throw chaos::CException(1, "Error creating priority socket", __FUNCTION__);
    //set socket identity
    err = zmq_setsockopt (socket_priority, ZMQ_IDENTITY, priority_identity.c_str(), priority_identity.size());
	if(err) throw chaos::CException(err, "Error setting priority socket option", __FUNCTION__);
    //set output queue dime
	err = zmq_setsockopt (socket_priority, ZMQ_SNDHWM, &output_buffer_dim, sizeof(int));
	if(err) throw chaos::CException(err, "Error setting ZMQ_SNDHWM on priority socket option", __FUNCTION__);
	
    ZMQDIOLAPP_ << "Allocating priority socket";
    socket_service = zmq_socket (zmq_context, ZMQ_DEALER);
    if(socket_service == NULL) throw chaos::CException(2, "Error creating service socket", __FUNCTION__);
    //set socket identity
    err = zmq_setsockopt(socket_service, ZMQ_IDENTITY, service_identity.c_str(), service_identity.size());
	if(err) throw chaos::CException(err, "Error setting service socket option", __FUNCTION__);
	//set output queue dime
	err = zmq_setsockopt (socket_service, ZMQ_SNDHWM, &output_buffer_dim, sizeof(int));
	if(err) throw chaos::CException(err, "Error setting ZMQ_SNDHWM on priority socket option", __FUNCTION__);

	
	err = zmq_socket_monitor(socket_priority, "inproc://monitor.req", ZMQ_EVENT_ALL);
	monitor_thread.reset(new boost::thread(boost::bind(&ZMQDirectIOClient::monitorWorker, this)));
	
    ZMQDIOLAPP_ << "Initialized";
}

//! Deinit the implementation
void ZMQDirectIOClient::deinit() throw(chaos::CException) {
    ZMQDIOLAPP_ << "deinitialization";
    ZMQDIOLDBG_ << "Acquiring lock";
    ZMQDirectIOClientWriteLock lock(mutex_socket_manipolation);
    ZMQDIOLDBG_ << "Write lock acquired";

	ZMQDIOLAPP_ << "Close monitor socket";
    ZMQBaseClass::closeSocketNoWhait(socket_monitor);
    //zmq_close(socket_service);
    socket_monitor = NULL;
	
    ZMQDIOLAPP_ << "Close priority socket";
    ZMQBaseClass::closeSocketNoWhait(socket_priority);
    //zmq_close(socket_priority);
    socket_priority = NULL;
    
    ZMQDIOLAPP_ << "Close service socket";
    ZMQBaseClass::closeSocketNoWhait(socket_service);
    //zmq_close(socket_service);
    socket_service = NULL;
	
    //destroy the zmq context
    ZMQDIOLAPP_ << "Destroyed zmq context";
    zmq_ctx_destroy(zmq_context);
    zmq_context = NULL;
    
    DirectIOClient::deinit();
    ZMQDIOLAPP_ << "Deinitialized";
}

//! check the connection with the endpoint for the two socket
void *ZMQDirectIOClient::monitorWorker() {
	zmq_event_t event;
    int err = 0;
	char addr[1025];
    void *s = zmq_socket (zmq_context, ZMQ_PAIR);
    assert (s);
    err = zmq_connect(s, "inproc://monitor.req");
    assert (err == 0);
    while (!read_msg(s, &event, addr)) {
        switch (event.event) {
			case ZMQ_EVENT_CONNECTED:
				ZMQDIOLDBG_ << "ZMQ_EVENT_CONNECTED:"<<addr;
				break;
			case ZMQ_EVENT_CONNECT_DELAYED:
				ZMQDIOLDBG_ << "ZMQ_EVENT_CONNECT_DELAYED:"<<addr;
				break;
			case ZMQ_EVENT_CONNECT_RETRIED:
				ZMQDIOLDBG_ << "ZMQ_EVENT_CONNECT_RETRIED:"<<addr;
				break;
			case ZMQ_EVENT_CLOSED:
				ZMQDIOLDBG_ << "ZMQ_EVENT_CLOSED:"<<addr;
				break;
			case ZMQ_EVENT_CLOSE_FAILED:
				ZMQDIOLDBG_ << "ZMQ_EVENT_CLOSE_FAILED:"<<addr;
				break;
			case ZMQ_EVENT_DISCONNECTED:
				ZMQDIOLDBG_ << "ZMQ_EVENT_DISCONNECTED:"<<addr;
				break;
			default:
				break;
        }
    }
    zmq_close (s);
    return NULL;
}
/*
void ZMQDirectIOClient::switchModeTo(DirectIOConnectionSpreadType::DirectIOConnectionSpreadType connection_mode) {
    int err = 0;
    std::string _priority_end_point;
	std::string _service_end_point;
    //check if are already on the same spred type
    //if(direct_io_spread_mode == current_spread_forwarding_type) return;
    //current_spread_forwarding_type = direct_io_spread_mode;
    
    //lock for write
	ZMQDIOLDBG_ << "Acquiring lock";
    ZMQDirectIOClientWriteLock lock(mutex_socket_manipolation);
    ZMQDIOLDBG_ << "Write lock acquired";
    switch (connection_mode) {
        case DirectIOConnectionSpreadType::DirectIORoundRobin: {
            ZMQDIOLDBG_ << "Switch mod to DirectIORoundRobin";
            std::vector< std::vector<std::string> > all_online_server;
            ServerFeeder::getAllOnlineServer(all_online_server);
            for (std::vector< std::vector<std::string> >::iterator i = all_online_server.begin();
                 i != all_online_server.end();
                 i++) {
                current_priority_endpoint = boost::str( boost::format("tcp://%1%") % (*i)[0]);
                current_service_endpoint = boost::str( boost::format("tcp://%1%") % (*i)[1]);
                
                ZMQDIOLDBG_ << "connect to priority endpoint " << current_priority_endpoint;
                err = zmq_connect(socket_priority, current_priority_endpoint.c_str());
                if(err) {
                    ZMQDIOLERR_ << "Error connecting priority socket to " << current_priority_endpoint;
                }
                
                ZMQDIOLDBG_ << "connect to service endpoint " << current_service_endpoint;
                err = zmq_connect(socket_service, current_service_endpoint.c_str());
                if(err) {
                    ZMQDIOLERR_ << "Error connecting service socket to " << current_service_endpoint;
                }
            }
            break;
        }
            
        case DirectIOConnectionSpreadType::DirectIOFailOver: {
            ZMQDIOLDBG_ << "Switch mod to DirectIOFailOver";
            //try connecting to first server
            ServerFeeder::getCurrentOnline(current_server_hash, _priority_end_point, _service_end_point);
            //connect the socket to server
            current_priority_endpoint = boost::str( boost::format("tcp://%1%") % _priority_end_point);
            current_service_endpoint = boost::str( boost::format("tcp://%1%") % _service_end_point);
            
            ZMQDIOLDBG_ << "connect to priority endpoint " << current_priority_endpoint;
            err = zmq_connect(socket_priority, current_priority_endpoint.c_str());
            if(err) {
                ZMQDIOLERR_ << "Error connecting priority socket to " << current_priority_endpoint;
            }
			//add monitor on priority socket
            ZMQDIOLDBG_ << "connect to service endpoint " << current_service_endpoint;
            err = zmq_connect(socket_service, current_service_endpoint.c_str());
            if(err) {
                ZMQDIOLERR_ << "Error connecting service socket to " << current_service_endpoint;
            }
            break;
        }
			
		default:
			break;
    }
    
}*/

// send the data to the server layer on priority channel
uint32_t ZMQDirectIOClient::sendPriorityData(DirectIODataPack *data_pack) {
    return writeToSocket(socket_priority, data_pack);
}

// send the data to the server layer on the service channel
uint32_t ZMQDirectIOClient::sendServiceData(DirectIODataPack *data_pack) {
    return writeToSocket(socket_service, data_pack);
}

uint32_t ZMQDirectIOClient::writeToSocket(void *socket, DirectIODataPack *data_pack) {
    assert(socket && data_pack);
	ZMQDirectIOClientReadLock lock(mutex_socket_manipolation);
	//send header
	int err = zmq_send(socket, &data_pack->header.dispatcher_raw_data, 4, ZMQ_SNDMORE);
	if(err == -1) {
		return err;
	}
	//send data
    return zmq_send(socket, data_pack->data, data_pack->data_size, ZMQ_DONTWAIT);

}

int ZMQDirectIOClient::addServer(std::string server_desc) {
	if(InizializableService::getServiceState() == ::chaos::utility::service_state_machine::InizializableServiceType::IS_DEINTIATED)
		return 0;
	ZMQDirectIOClientWriteLock lock(mutex_socket_manipolation);
	int err = 0;
	std::string priority_endpoint;
	std::string service_endpoint;
	std::string url;
	ServerFeeder::decoupleServerDescription(server_desc, priority_endpoint, service_endpoint);
	
	url = boost::str( boost::format("tcp://%1%") % priority_endpoint);
	ZMQDIOLDBG_ << "connect to priority endpoint " << url;
	err = zmq_connect(socket_priority, url.c_str());
	if(err) {
		ZMQDIOLERR_ << "Error connecting priority socket to " << priority_endpoint;
		return err;
	}
	
	//add monitor on priority socket
	url = boost::str( boost::format("tcp://%1%") % service_endpoint);
	ZMQDIOLDBG_ << "connect to service endpoint " << url;
	err = zmq_connect(socket_service, url.c_str());
	if(err) {
		ZMQDIOLERR_ << "Error connecting service socket to " << service_endpoint;
		return err;
	}
	return 0;
}

int ZMQDirectIOClient::removeServer(std::string server_desc) {
	if(InizializableService::getServiceState() == ::chaos::utility::service_state_machine::InizializableServiceType::IS_DEINTIATED)
		return 0;
	ZMQDirectIOClientWriteLock lock(mutex_socket_manipolation);
	int err = 0;
	std::string priority_endpoint;
	std::string service_endpoint;
	std::string url;
	ServerFeeder::decoupleServerDescription(server_desc, priority_endpoint, service_endpoint);
	
	url = boost::str( boost::format("tcp://%1%") % priority_endpoint);
	ZMQDIOLDBG_ << "connect to priority endpoint " << url;
	err = zmq_disconnect(socket_priority, url.c_str());
	if(err) {
		ZMQDIOLERR_ << "Error connecting priority socket to " << priority_endpoint;
		return err;
	}
	
	//add monitor on priority socket
	url = boost::str( boost::format("tcp://%1%") % service_endpoint);
	ZMQDIOLDBG_ << "connect to service endpoint " << url;
	err = zmq_disconnect(socket_service, url.c_str());
	if(err) {
		ZMQDIOLERR_ << "Error connecting service socket to " << service_endpoint;
		return err;
	}
	return 0;
}


