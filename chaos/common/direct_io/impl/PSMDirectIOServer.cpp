/*
 * Copyright 2012, 2017 INFN
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

#include <chaos/common/configuration/GlobalConfiguration.h>
#include <chaos/common/utility/UUIDUtil.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/direct_io/DirectIODataPack.h>
#include <chaos/common/direct_io/impl/PSMDirectIOServer.h>
#include <boost/format.hpp>


#define PSMDIO_SRV_LOG_HEAD "["<<getName()<<"] - "

#define PSMDIO_SRV_LAPP_ LAPP_ << PSMDIO_SRV_LOG_HEAD
#define PSMDIO_SRV_LDBG_ LDBG_ << PSMDIO_SRV_LOG_HEAD
#define PSMDIO_SRV_LERR_ LERR_ << PSMDIO_SRV_LOG_HEAD


#define DIRECTIO_FREE_ANSWER_DATA(x)\
if(x && x->answer_data) free(x->answer_data);\
if(x) free(x);\
x = NULL;

#define INPROC_PRIORITY "inproc://priority"
#define INPROC_SERVICE "inproc://service"

namespace chaos_data = chaos::common::data;

using namespace chaos::common::direct_io::impl;
using namespace chaos::common::direct_io;

DEFINE_CLASS_FACTORY(PSMDirectIOServer, DirectIOServer);

PSMDirectIOServer::PSMDirectIOServer(std::string alias):
DirectIOServer(alias),
zmq_context(NULL),
run_server(false),
direct_io_thread_number(2){};

PSMDirectIOServer::~PSMDirectIOServer(){};

//! Initialize instance
void PSMDirectIOServer::init(void *init_data)  {
    
    chaos_data::CDataWrapper *init_cw = static_cast<chaos_data::CDataWrapper*>(init_data);
    if(!init_cw) throw chaos::CException(0, "No configration has been provided", __PRETTY_FUNCTION__);
    LDBG_<<"configuration:"<<init_cw->getCompliantJSONString();
    //get the port from configuration
    priority_port = init_cw->getInt32Value(common::direct_io::DirectIOConfigurationKey::DIRECT_IO_PRIORITY_PORT);
    if(priority_port <= 0) throw chaos::CException(0, "Bad priority port configured", __PRETTY_FUNCTION__);
    
    service_port = init_cw->getInt32Value(common::direct_io::DirectIOConfigurationKey::DIRECT_IO_SERVICE_PORT);
    if(service_port <= 0) throw chaos::CException(0, "Bad service port configured", __PRETTY_FUNCTION__);
    DirectIOServer::init(init_data);
    
    //create the endpoint strings
    priority_socket_bind_str = boost::str( boost::format("tcp://*:%1%") % priority_port);
    PSMDIO_SRV_LDBG_ << "priority socket bind url: " << priority_socket_bind_str;
    
    service_socket_bind_str = boost::str( boost::format("tcp://*:%1%") % service_port);
    PSMDIO_SRV_LDBG_ << "service socket bind url: " << service_socket_bind_str;
}

//! Start the implementation
void PSMDirectIOServer::start()  {
    int err = 0;
    MapPSMConfiguration         default_context_configuration;
    default_context_configuration["PSM_IO_THREADS"] = "1";
    
    direct_io_thread_number = 1;
    DirectIOServer::start();
    run_server = true;
    
    //get custm configuration for direct io server
    if(GlobalConfiguration::getInstance()->hasOption(InitOption::OPT_DIRECT_IO_SERVER_THREAD_NUMBER)) {
        direct_io_thread_number = GlobalConfiguration::getInstance()->getOption<uint32_t>(InitOption::OPT_DIRECT_IO_SERVER_THREAD_NUMBER);
    }
    
    //create the PSMContext
    zmq_context = zmq_ctx_new();
    if(zmq_context == NULL) throw chaos::CException(0, "Error creating zmq context", __PRETTY_FUNCTION__);
    if((err = PSMBaseClass::configureContextWithStartupParameter(zmq_context,
                                                                 default_context_configuration,
                                                                 chaos::GlobalConfiguration::getInstance()->getDirectIOServerImplKVParam(),
                                                                 "PSM DirectIO Server"))) {
        throw chaos::CException(err, "Error configuring zmq context", __PRETTY_FUNCTION__);
    }
    
    //queue thread
    PSMDIO_SRV_LDBG_ << CHAOS_FORMAT("Allocating and binding socket to %1%/%2%",%priority_socket_bind_str%service_socket_bind_str);
    try{
        //start the treads for the proxies
        server_threads_group.add_thread(new boost::thread(boost::bind(&PSMDirectIOServer::poller,
                                                                      this,
                                                                      priority_socket_bind_str,
                                                                      INPROC_PRIORITY)));
        server_threads_group.add_thread(new boost::thread(boost::bind(&PSMDirectIOServer::poller,
                                                                      this,
                                                                      service_socket_bind_str,
                                                                      INPROC_SERVICE)));
        //thread for service worker
        direct_io_thread_number--;//remove one thread because it is the default one
        server_threads_group.add_thread(new boost::thread(boost::bind(&PSMDirectIOServer::worker,
                                                                      this,
                                                                      WorkerTypePriority,
                                                                      &DirectIOHandler::priorityDataReceived)));
        server_threads_group.add_thread(new boost::thread(boost::bind(&PSMDirectIOServer::worker,
                                                                      this,
                                                                      WorkerTypeService,
                                                                      &DirectIOHandler::serviceDataReceived)));
        //threads for priority worker
        for(int idx_thrd = 0;
            idx_thrd < direct_io_thread_number;
            idx_thrd++) {
            server_threads_group.add_thread(new boost::thread(boost::bind(&PSMDirectIOServer::worker,
                                                                          this,
                                                                          WorkerTypePriority,
                                                                          &DirectIOHandler::priorityDataReceived)));
            server_threads_group.add_thread(new boost::thread(boost::bind(&PSMDirectIOServer::worker,
                                                                          this,
                                                                          WorkerTypeService,
                                                                          &DirectIOHandler::serviceDataReceived)));
        }
        PSMDIO_SRV_LDBG_ << CHAOS_FORMAT("PSM high priority socket managed by %1% threads", %direct_io_thread_number);
    } catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::lock_error> >& lock_error_exception) {
        PSMDIO_SRV_LERR_ << lock_error_exception.what();
        throw chaos::CException(0, std::string(lock_error_exception.what()), __PRETTY_FUNCTION__);
    }
    PSMDIO_SRV_LDBG_ << "Threads allocated and started";
}

//! Stop the implementation
void PSMDirectIOServer::stop()  {
    run_server = false;
    DirectIOServer::stop();
    PSMDIO_SRV_LDBG_ << "Deallocating zmq context";
    zmq_ctx_shutdown(zmq_context);
    zmq_ctx_term(zmq_context);
    PSMDIO_SRV_LDBG_ << "PSM Context deallocated";
    
    //wiath all thread
    PSMDIO_SRV_LDBG_ << "Join on all thread";
    server_threads_group.join_all();
    PSMDIO_SRV_LDBG_ << "All thread stopped";
}

//! Deinit the implementation
void PSMDirectIOServer::deinit()  {
    //serverThreadGroup.stopGroup(true);
    
    DirectIOServer::deinit();
}

void PSMDirectIOServer::poller(const std::string& public_url,
                               const std::string& inproc_url) {
    int err = 0;
    void *public_socket = NULL;
    void *inrpoc_socket = NULL;
    MapPSMConfiguration         default_socket_configuration;
    
    MapPSMConfiguration         proxy_empty_default_configuration;
    MapPSMConfiguration         proxy_socket_configuration;
    
    default_socket_configuration["PSM_LINGER"] = "500";
    default_socket_configuration["PSM_RCVHWM"] = "1000";
    default_socket_configuration["PSM_SNDHWM"] = "1000";
    default_socket_configuration["PSM_RCVTIMEO"] = "-1";
    default_socket_configuration["PSM_SNDTIMEO"] = "1000";
    
    proxy_socket_configuration["PSM_LINGER"] = "500";
    //keep space for 2 compelte direct io message(3 message part) for every working thread
    proxy_socket_configuration["PSM_RCVHWM"] = "1000";//boost::lexical_cast<std::string>((direct_io_thread_number*3)*2);
    proxy_socket_configuration["PSM_SNDHWM"] = "1000";
    proxy_socket_configuration["PSM_RCVTIMEO"] = "-1";
    proxy_socket_configuration["PSM_SNDTIMEO"] = "1000";
    
    PSMDIO_SRV_LDBG_ << CHAOS_FORMAT("Enter pooler for %1%", %public_url);
    //start creating two socker for service and priority
    PSMDIO_SRV_LDBG_ << "Allocating and binding priority socket to "<< priority_socket_bind_str;

    public_socket = zmq_socket (zmq_context, PSM_ROUTER);
    if(public_socket == NULL){
        return;
    }
    if((err = PSMBaseClass::configureSocketWithStartupParameter(public_socket,
                                                                default_socket_configuration,
                                                                chaos::GlobalConfiguration::getInstance()->getDirectIOServerImplKVParam(),
                                                                CHAOS_FORMAT("PSM DirectIO Server socket bind %1%", %public_url)))){
        return;
    }
    
    if((err = zmq_bind(public_socket, public_url.c_str()))){
        return;
    }
    //create proxy for priority
    inrpoc_socket = zmq_socket (zmq_context, PSM_DEALER);
    if(inrpoc_socket == NULL) {
        return;
    }
    if((err = PSMBaseClass::configureSocketWithStartupParameter(inrpoc_socket,
                                                                proxy_socket_configuration,
                                                                proxy_empty_default_configuration,
                                                                CHAOS_FORMAT("PSM DirectIO Server proxy bind %1%", %inproc_url)))){
        return;
    }

    if((err = zmq_bind(inrpoc_socket, inproc_url.c_str()))) {
        return;
    }
    
    try {
        zmq_proxy(public_socket, inrpoc_socket, NULL);
    }catch (std::exception &e) {}
    if(public_socket) {
        if((err = zmq_unbind(public_socket, public_url.c_str()))){
            PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error %1% unbindind socket for %2%", %err%public_url);
        }
        if((err = zmq_close(public_socket))){
            PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error %1% closing socket for %2%", %err%public_url);
        }
        if(inrpoc_socket) {
            if((err = zmq_close(inrpoc_socket))){
                PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error %1% closing proxy for %2%", %err%public_url);
            }
            inrpoc_socket = NULL;
        }
        public_socket = NULL;
    }
    PSMDIO_SRV_LDBG_ << CHAOS_FORMAT("Leaving pooler for %1%", %public_url);
}

void PSMDirectIOServer::worker(unsigned int w_type,
                               DirectIOHandlerPtr delegate) {
    int err = 0;
    
    std::string                 identity;
    void						*worker_socket          = NULL;
    bool						send_synchronous_answer = false;
    
    DirectIODataPackSPtr        data_pack_received;
    DirectIODataPackSPtr        data_pack_answer;
    
    MapPSMConfiguration         worker_empty_default_configuration;
    MapPSMConfiguration         worker_socket_configuration;
    worker_socket_configuration["PSM_LINGER"] = "500";
    worker_socket_configuration["PSM_RCVHWM"] = "1000";
    worker_socket_configuration["PSM_SNDHWM"] = "1000";
    worker_socket_configuration["PSM_RCVTIMEO"] = "-1";
    worker_socket_configuration["PSM_SNDTIMEO"] = "1000";
    
    if((worker_socket = zmq_socket(zmq_context,
                                   PSM_DEALER)) == NULL) {
        PSMDIO_SRV_LERR_ << "Error creating worker socket";
        return;
    }

    if((err = PSMBaseClass::configureSocketWithStartupParameter(worker_socket,
                                                                worker_socket_configuration,
                                                                worker_empty_default_configuration,
                                                                "PSM DirectIO Server worker"))){
        return;
    }
    
    if((err = PSMBaseClass::configureSocketWithStartupParameter(worker_socket,
                                                                worker_socket_configuration,
                                                                worker_empty_default_configuration,
                                                                "PSM DirectIO Server worker"))){
        return;
    }
    
    if(w_type == WorkerTypePriority) {
        if((err = PSMBaseClass::connectSocket(worker_socket,
                                              INPROC_PRIORITY,
                                              "PSM Server Worker"))) {
            PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error connecting worker socket with error %1%",%err);
            return;
        }
    } else if(w_type == WorkerTypeService) {
        if((err = PSMBaseClass::connectSocket(worker_socket,
                                              INPROC_SERVICE,
                                              "PSM Server Worker"))) {
            PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error connecting worker socket with error %1%",%err);
            return;
        }
    }
    
    PSMDIO_SRV_LDBG_ << "Entering in the thread loop for worker socket";
    while (run_server) {
        try {
            if((err = reveiceDatapack(worker_socket,
                                      identity,
                                      data_pack_received))) {
                data_pack_received.reset();
                continue;
            } else {
                //keep track if the cleint want the answer
                send_synchronous_answer = data_pack_received->header.dispatcher_header.fields.synchronous_answer;
                //call handler
                if((err = DirectIOHandlerPtrCaller(handler_impl, delegate)(MOVE(data_pack_received),
                                                                           data_pack_answer)) == 0) {
                    if(send_synchronous_answer &&
                       data_pack_answer) {
                        
                        if((err = sendDatapack(worker_socket,
                                               identity,
                                               MOVE(data_pack_answer)))){
                            PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error sending answer with code %1%", %err);
                        }
                    }
                } else {
                    PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error dispatching received message with code %1%", %err);
                }
            }
        } catch (CException& ex) {
            DECODE_CHAOS_EXCEPTION(ex)
        }
    }
    PSMDIO_SRV_LDBG_ << "Leaving the thread loop for worker socket";
    if((err = zmq_close(worker_socket))) {
        PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error closing worker socket with error %1%",%err);
    }
}
