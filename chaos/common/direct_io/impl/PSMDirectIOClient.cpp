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
#include <chaos/common/utility/UUIDUtil.h>
#include <chaos/common/utility/InetUtility.h>
#include <chaos/common/direct_io/impl/PSMDirectIOClient.h>
#include <chaos/common/data/cache/FastHash.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <string.h>
#include <assert.h>     /* assert */

#define PSMDIO_LOG_HEAD "["<<getName()<<"] - "

#define PSMDIOLAPP_ LAPP_ << PSMDIO_LOG_HEAD
#define PSMDIOLDBG_ LDBG_ << PSMDIO_LOG_HEAD << __FUNCTION__ << " - "
#define PSMDIOLERR_ LERR_ << PSMDIO_LOG_HEAD

using namespace chaos::common::utility;

using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::impl;

typedef boost::unique_lock<boost::shared_mutex>	PSMDirectIOClientWriteLock;
typedef boost::shared_lock<boost::shared_mutex> PSMDirectIOClientReadLock;

DEFINE_CLASS_FACTORY(PSMDirectIOClient, DirectIOClient);

//------------------------------STATIC METHOD---------------------------------


PSMDirectIOClient::PSMDirectIOClient(std::string alias):
DirectIOClient(alias),
priority_port(0),
service_port(0),
thread_run(false),
zmq_context(NULL){};

PSMDirectIOClient::~PSMDirectIOClient(){};

//! Initialize instance
void PSMDirectIOClient::init(void *init_data)  {
    int err = 0;
    MapPSMConfiguration default_configuration;
    default_configuration["PSM_IO_THREADS"] = "1";
    
    DirectIOClient::init(init_data);
    PSMDIOLDBG_ << "Allocating zmq context";
    thread_run= true;
    zmq_context = zmq_ctx_new();
    if(zmq_context == NULL) throw chaos::CException(0, "Error creating zmq context", __FUNCTION__);
    if((err = PSMBaseClass::configureContextWithStartupParameter(zmq_context,
                                                                 default_configuration,
                                                                 chaos::GlobalConfiguration::getInstance()->getDirectIOClientImplKVParam(),
                                                                 "PSM DirectIO Client"))) {
        throw chaos::CException(2, "Error configuring service socket", __FUNCTION__);
    }
    
    
    PSMDIOLDBG_ << "Inizilizing zmq implementation with zmq lib version = " << PSM_VERSION;
    PSMDIOLDBG_ << "Set number of thread for the contex";
    zmq_ctx_set(zmq_context, PSM_IO_THREADS, 2);
    
    PSMDIOLDBG_ << "Initialized";
}

//! Deinit the implementation
void PSMDirectIOClient::deinit()  {
    int err = 0;
    //remove all active connection (never need to be exists at this step)
    map_connections.clearElement();
    //destroy the zmq context
    PSMDIOLDBG_ << "Destroing zmq context";
    thread_run = false;
    err = zmq_ctx_destroy(zmq_context);
    if(err) PSMDIOLERR_ << "Error closing context";
    //monitor_thread_group.join_all();
    
    zmq_context = NULL;
    PSMDIOLDBG_ << "PSM context destroyed";
    DirectIOClient::deinit();
}

DirectIOClientConnection *PSMDirectIOClient::_getNewConnectionImpl(std::string server_description,
                                                                   uint16_t endpoint) {
    //allocate client
    PSMDirectIOClientConnection *connection = new PSMDirectIOClientConnection(zmq_context,
                                                                              server_description,
                                                                              endpoint);
    if(connection == NULL) return NULL;
    try{
        InizializableService::initImplementation(connection, NULL, "PSMDirectIOClientConnection", __PRETTY_FUNCTION__);
        //register client with the hash of the xzmq decoded endpoint address (tcp://ip:port)
        DEBUG_CODE(PSMDIOLDBG_ << "Register client for " << server_description << " with zmq decoded hash " << connection->getUniqueUUID();)
        map_connections.registerElement(connection->getUniqueUUID(), connection);
    } catch (...) {
        PSMDIOLERR_ << CHAOS_FORMAT("We got error initilizing connection to %1%:%2% so we goning to deinitilize it an return NULL channel", %server_description%endpoint);
        //in case of error
        CHAOS_NOT_THROW(InizializableService::deinitImplementation(connection, "PSMDirectIOClientConnection", __PRETTY_FUNCTION__););
        connection = NULL;
    }
    return connection;
}

void PSMDirectIOClient::_releaseConnectionImpl(DirectIOClientConnection *connection_to_release) {
    PSMDirectIOClientConnection *conn=reinterpret_cast<PSMDirectIOClientConnection*>(connection_to_release);
    if(!conn) return;
    CHAOS_NOT_THROW(InizializableService::deinitImplementation(conn, "PSMDirectIOClientConnection", __PRETTY_FUNCTION__););
    //CHAOS_ASSERT(conn->monitor_info)
    //stop the monitor
    DEBUG_CODE(PSMDIOLDBG_ << "Release the connection for: " << connection_to_release->getServerDescription() <<" ptr:"<<std::hex<<(uint64_t)connection_to_release;)
    map_connections.deregisterElementKey(conn->getUniqueUUID());
    delete(connection_to_release);
}

void PSMDirectIOClient::freeObject(const DCKeyObjectContainer::TKOCElement& element) {
    if(!element.element) return;
    DirectIOClientConnection *connection = element.element;
    DEBUG_CODE(PSMDIOLDBG_ << "Autorelease connection for " << connection->getServerDescription();)
    releaseConnection(connection);
}
