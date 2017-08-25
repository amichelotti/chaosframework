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

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include <chaos/common/network/NetworkBroker.h>
#include <chaos/common/io/IODirectIODriver.h>
#include <chaos/common/chaos_constants.h>
#include <chaos/common//global.h>
#include <chaos/common/utility/InizializableService.h>
#include <chaos/common/utility/UUIDUtil.h>

#include <chaos/common/direct_io/impl/ZMQDirectIOClientConnection.h>

#define IODirectIODriver_LOG_HEAD "[IODirectIODriver] - "

#define IODirectIODriver_LINFO_ INFO_LOG(IODirectIODriver)
#define IODirectIODriver_DLDBG_ DBG_LOG(IODirectIODriver)
#define IODirectIODriver_LERR_ ERR_LOG(IODirectIODriver)

using namespace chaos;
using namespace chaos::common::io;
using namespace chaos::common::utility;

using namespace std;
using namespace boost;
using namespace boost::algorithm;

namespace chaos_data = chaos::common::data;
namespace chaos_dio = chaos::common::direct_io;
namespace chaos_dio_channel = chaos::common::direct_io::channel;

DEFINE_CLASS_FACTORY(IODirectIODriver, IODataDriver);

//using namespace memcache;
IODirectIODriver::IODirectIODriver(const std::string& alias):
NamedService(alias),
current_endpoint_p_port(0),
current_endpoint_s_port(0),
current_endpoint_index(0),
connectionFeeder(alias, this),
uuid(UUIDUtil::generateUUIDLite()),
shutting_down(false){
    //clear
    std::memset(&init_parameter, 0, sizeof(IODirectIODriverInitParam));
    
    device_server_channel = NULL;
}

IODirectIODriver::~IODirectIODriver() {
}

void IODirectIODriver::setDirectIOParam(IODirectIODriverInitParam& _init_parameter) {
    //store the configuration
    //init_parameter = _init_parameter;
}

void IODirectIODriver::init(void *_init_parameter) throw(CException) {
    shutting_down = false;
    IODataDriver::init(_init_parameter);
    
    IODirectIODriver_LINFO_ << "Check init parameter";
    
    init_parameter.client_instance = NetworkBroker::getInstance()->getSharedDirectIOClientInstance();
    //if(!init_parameter.client_instance) throw CException(-1, "No client configured", __PRETTY_FUNCTION__);
    
    init_parameter.endpoint_instance = NetworkBroker::getInstance()->getDirectIOServerEndpoint();
    if(!init_parameter.endpoint_instance) throw CException(-1, "No endpoint configured", __PRETTY_FUNCTION__);
    
    //initialize client
    //InizializableService::initImplementation(init_parameter.client_instance, _init_parameter, init_parameter.client_instance->getName(), __PRETTY_FUNCTION__);
    
    //get the client and server channel
    IODirectIODriver_LINFO_ << "Allocate the default device server channel";
    device_server_channel = (chaos_dio_channel::DirectIODeviceServerChannel *)init_parameter.endpoint_instance->getNewChannelInstance("DirectIODeviceServerChannel");
    device_server_channel->setHandler(this);
    
    //store endpoint idnex for fast access
    current_endpoint_p_port = init_parameter.endpoint_instance->getPublicServerInterface()->getPriorityPort();
    current_endpoint_s_port = init_parameter.endpoint_instance->getPublicServerInterface()->getServicePort();
    current_endpoint_index = init_parameter.endpoint_instance->getRouteIndex();
    IODirectIODriver_LINFO_ << "Our receiving priority port is " << current_endpoint_p_port << " and enpoint is " <<current_endpoint_index;
    
}

void IODirectIODriver::deinit() throw(CException) {
    shutting_down = true;
    IODirectIODriver_LINFO_ << "Remove active query";
    //lock all  internal resource that can be effetted by
    boost::unique_lock<boost::shared_mutex> wmap_loc(map_query_future_mutex);
    
    //scan all remained query
    for(std::map<string, QueryCursor*>::iterator it = map_query_future.begin();
        it != map_query_future.end();
        it++) {
        releaseQuery(it->second);
    }
    map_query_future.clear();
    
    //remove all url and service
    IODirectIODriver_LINFO_ << "Remove all urls";
    connectionFeeder.clear();
    
    //deinitialize server channel
    if(device_server_channel) {
        //remove me as handler
        device_server_channel->setHandler(NULL);
        init_parameter.endpoint_instance->releaseChannelInstance(device_server_channel);
    }
    
    if(init_parameter.endpoint_instance) {
        NetworkBroker::getInstance()->releaseDirectIOServerEndpoint(init_parameter.endpoint_instance);
    }
    IODataDriver::deinit();
}

void IODirectIODriver::storeRawData(const std::string& key,
                                    chaos::common::data::SerializationBuffer *serialization,
                                    DataServiceNodeDefinitionType::DSStorageType storage_type)  throw(CException) {
    CHAOS_ASSERT(serialization)
    int err = 0;
    boost::shared_lock<boost::shared_mutex> rl(mutext_feeder);
    //if(next_client->connection->getState() == chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionEventConnected)
    IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
    serialization->disposeOnDelete = !next_client;
    if(next_client) {
        //free the packet
        serialization->disposeOnDelete = false;
        if((err = (int)next_client->device_client_channel->storeAndCacheDataOutputChannel(key,
                                                                                          (void*)serialization->getBufferPtr(),
                                                                                          (uint32_t)serialization->getBufferLen(),
                                                                                          storage_type))) {
            IODirectIODriver_LERR_ << "Error storing data into data service "<<next_client->connection->getServerDescription()<<" with code:" << err;
        }
    } else {
        DEBUG_CODE(IODirectIODriver_DLDBG_ << "No available socket->loose packet");
    }
    delete(serialization);
}


void IODirectIODriver::storeHealthData(const std::string& key,
                                       chaos_data::CDataWrapper& dataToStore,
                                       DataServiceNodeDefinitionType::DSStorageType storage_type) throw(CException) {
    int err = 0;
    try{
        boost::shared_lock<boost::shared_mutex> rl(mutext_feeder);
        IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
        
        ChaosUniquePtr<chaos::common::data::SerializationBuffer> serialization(dataToStore.getBSONData());
        
        if(next_client &&
           serialization.get()) {
            serialization->disposeOnDelete = false;
            if((err = (int)next_client->device_client_channel->storeAndCacheHealthData(key,
                                                                                       (void*)serialization->getBufferPtr(),
                                                                                       (uint32_t)serialization->getBufferLen(),
                                                                                       storage_type))) {
                IODirectIODriver_LERR_ << "Error storing health data into data service "<<next_client->connection->getServerDescription()<<" with code:" << err;
            }
        } else {
            DEBUG_CODE(IODirectIODriver_DLDBG_ << "No available socket->loose packet");
        }
    }catch(bson::AssertionException& bson_assert){
        IODirectIODriver_LERR_ << CHAOS_FORMAT("bson assertion [%1%]", %
                                               bson_assert.toString());
    }
}

char* IODirectIODriver::retriveRawData(const std::string& key, size_t *dim)  throw(CException) {
    char* result = NULL;
    boost::shared_lock<boost::shared_mutex> rl(mutext_feeder);
    IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
    if(!next_client) return NULL;
    
    uint32_t size =0;
    int err = (int)next_client->device_client_channel->requestLastOutputData(key, (void**)&result, size);
    if(err) {
        IODirectIODriver_LERR_ << "Error retriving data from data service "<<next_client->connection->getServerDescription()<< " with code:" << err;
    } else {
        *dim = (size_t)size;
    }
    return result;
}

int IODirectIODriver::retriveMultipleData(const ChaosStringVector& key,
                                          chaos::common::data::VectorCDWShrdPtr& result)  throw(CException) {
    boost::shared_lock<boost::shared_mutex> rl(mutext_feeder);
    IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
    if(!next_client) return -1;
    
    int err = (int)next_client->device_client_channel->requestLastOutputData(key,
                                                                             result);
    if(err) {
        IODirectIODriver_LERR_ << "Error retriving data from data service "<<next_client->connection->getServerDescription()<< " with code:" << err;
    }
    return err;
}

int IODirectIODriver::removeData(const std::string& key,
                                 uint64_t start_ts,
                                 uint64_t end_ts) throw(CException) {
    boost::shared_lock<boost::shared_mutex> rl(mutext_feeder);
    IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
    if(!next_client) return -1;
    
    int err = (int)next_client->device_client_channel->deleteDataCloud(key,
                                                                       start_ts,
                                                                       end_ts);
    if(err) {
        IODirectIODriver_LERR_ << CHAOS_FORMAT("Error removing data from data service %1% with code %2% for key %3%",%next_client->connection->getServerDescription()%err%key);
    }
    return err;
}

int IODirectIODriver::loadDatasetTypeFromSnapshotTag(const std::string& restore_point_tag_name,
                                                     const std::string& key,
                                                     uint32_t dataset_type,
                                                     chaos_data::CDataWrapper **cdatawrapper_handler) {
    int err = 0;
    boost::shared_lock<boost::shared_mutex> rl(mutext_feeder);
    IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(connectionFeeder.getService());
    *cdatawrapper_handler=NULL;
    if(!next_client) return 0;
    chaos_dio_channel::DirectIOSystemAPIGetDatasetSnapshotResultPtr snapshot_result = NULL;
    if((err = (int)next_client->system_client_channel->getDatasetSnapshotForProducerKey(restore_point_tag_name,
                                                                                        key,
                                                                                        dataset_type,
                                                                                        &snapshot_result))) {
        IODirectIODriver_LERR_ << "Error loading the dataset type:"<<dataset_type<< " for key:" << key << " from restor point:" <<restore_point_tag_name;
    } else {
        if(snapshot_result && snapshot_result->channel_data) {
            //we have the dataaset
            try {
                *cdatawrapper_handler = new chaos_data::CDataWrapper((const char*)snapshot_result->channel_data);
                IODirectIODriver_LINFO_ << "Got dataset type:"<<dataset_type<< " for key:" << key << " from snapshot tag:" <<restore_point_tag_name;
                
            } catch (std::exception& ex) {
                IODirectIODriver_LERR_ << "Error deserializing the dataset type:"<<dataset_type<< " for key:" << key << " from snapshot tag:" <<restore_point_tag_name << " with error:" << ex.what();
            } catch (...) {
                IODirectIODriver_LERR_ << "Error deserializing the dataset type:"<<dataset_type<< " for key:" << key << " from snapshot tag:" <<restore_point_tag_name;
            }
            free(snapshot_result->channel_data);
        }
    }
    
    //delete the received result if there was one
    if(snapshot_result) free(snapshot_result);
    return err;
}

void IODirectIODriver::addServerURL(const std::string& url) {
    boost::unique_lock<boost::shared_mutex>(mutext_feeder);
    if(!common::direct_io::DirectIOClient::checkURL(url)) {
        IODirectIODriver_LERR_ << "Url " << url << " non well formed";
        return;
    }
    IODirectIODriver_LINFO_ << "Adding url" << url;
    //add new url to connection feeder
    connectionFeeder.addURL(chaos::common::network::URL(url));
}

chaos::common::data::CDataWrapper* IODirectIODriver::updateConfiguration(chaos::common::data::CDataWrapper* newConfigration) {
    //lock the feeder access
    boost::unique_lock<boost::shared_mutex> rl(mutext_feeder);
    //checkif someone has passed us the device indetification
    if(newConfigration->hasKey(DataServiceNodeDefinitionKey::DS_DIRECT_IO_FULL_ADDRESS_LIST)){
        IODirectIODriver_LINFO_ << "Get the DataManager LiveData address value";
        ChaosUniquePtr<chaos::common::data::CMultiTypeDataArrayWrapper> liveMemAddrConfig(newConfigration->getVectorValue(DataServiceNodeDefinitionKey::DS_DIRECT_IO_FULL_ADDRESS_LIST));
        size_t numerbOfserverAddressConfigured = liveMemAddrConfig->size();
        for ( int idx = 0; idx < numerbOfserverAddressConfigured; idx++ ){
            string serverDesc = liveMemAddrConfig->getStringElementAtIndex(idx);
            if(!common::direct_io::DirectIOClient::checkURL(serverDesc)) {
                IODirectIODriver_DLDBG_ << "Data proxy server description " << serverDesc << " non well formed";
                continue;
            }
            if(connectionFeeder.hasURL(serverDesc)) {
                IODirectIODriver_LERR_ << "Data proxy server description " << serverDesc << " is laredy instaleld in driver";
                continue;
            }
            //add new url to connection feeder
            connectionFeeder.addURL(chaos::common::network::URL(serverDesc));
        }
    }
    return NULL;
}

void IODirectIODriver::disposeService(void *service_ptr) {
    if(!service_ptr) return;
    IODirectIODriverClientChannels	*next_client = static_cast<IODirectIODriverClientChannels*>(service_ptr);
    //remove me as handler before delete all other this so anymore receive event
    next_client->connection->setEventHandler(NULL);
    if(next_client->system_client_channel) next_client->connection->releaseChannelInstance(next_client->system_client_channel);
    
    if(next_client->device_client_channel) {
        next_client->connection->releaseChannelInstance(next_client->device_client_channel);
    }
    
    init_parameter.client_instance->releaseConnection(next_client->connection);
    delete(next_client);
}

void* IODirectIODriver::serviceForURL(const common::network::URL& url, uint32_t service_index) {
    IODirectIODriver_LINFO_ << "try to add connection for " << url.getURL();
    IODirectIODriverClientChannels * clients_channel = NULL;
    chaos_direct_io::DirectIOClientConnection *tmp_connection = init_parameter.client_instance->getNewConnection(url.getURL());
    if(tmp_connection) {
        clients_channel = new IODirectIODriverClientChannels();
        clients_channel->connection = tmp_connection;
        
        //allocate the client channel
        clients_channel->device_client_channel = (chaos_dio_channel::DirectIODeviceClientChannel*)tmp_connection->getNewChannelInstance("DirectIODeviceClientChannel");
        if(!clients_channel->device_client_channel) {
            IODirectIODriver_LERR_ << "Error creating client device channel for " << url.getURL();
            
            //release conenction
            init_parameter.client_instance->releaseConnection(tmp_connection);
            
            //relase struct
            delete(clients_channel);
            return NULL;
        }
        
        clients_channel->system_client_channel = (chaos_dio_channel::DirectIOSystemAPIClientChannel*)tmp_connection->getNewChannelInstance("DirectIOSystemAPIClientChannel");
        if(!clients_channel->system_client_channel) {
            IODirectIODriver_LERR_ << "Error creating client system api channel for " << url.getURL();
            
            //releasing device channel
            tmp_connection->releaseChannelInstance(clients_channel->device_client_channel);
            
            //release connection
            init_parameter.client_instance->releaseConnection(tmp_connection);
            //relase struct
            delete(clients_channel);
            return NULL;
        }
        //set the answer information
        clients_channel->device_client_channel->setAnswerServerInfo(current_endpoint_p_port, current_endpoint_s_port, current_endpoint_index);
        
        //set this driver instance as event handler for connection
        clients_channel->connection->setEventHandler(this);
        clients_channel->connection->setCustomStringIdentification(boost::lexical_cast<std::string>(service_index));
    } else {
        IODirectIODriver_LERR_ << "Error creating client connection for " << url.getURL();
    }
    IODirectIODriver_LINFO_ << "connection for " << url.getURL() << " added succesfully";
    return clients_channel;
}

void IODirectIODriver::handleEvent(chaos_direct_io::DirectIOClientConnection *client_connection,
                                   chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionStateType event) {
    if(shutting_down) return;
    try {
        uint32_t service_index = boost::lexical_cast<uint32_t>(client_connection->getCustomStringIdentification());
        switch(event) {
            case chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionEventConnected:
                DEBUG_CODE(IODirectIODriver_LINFO_ << "Manage Connected event to service with index " << service_index << " and url" << client_connection->getURL();)
                connectionFeeder.setURLOnline(service_index);
                break;
                
            case chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionEventDisconnected:
                if(connectionFeeder.isOnline(service_index)){
                    DEBUG_CODE(IODirectIODriver_LINFO_ << "Manage Disconnected event for service with index " << service_index << " and url" << client_connection->getURL();)
                    connectionFeeder.setURLOffline(service_index);
                }
                break;
        }
    } catch(...){
        IODirectIODriver_LERR_ << "exception handling event identification:" << client_connection->getCustomStringIdentification() << " and url:" << client_connection->getURL();
    }
}

QueryCursor *IODirectIODriver::performQuery(const std::string& key,
                                            uint64_t start_ts,
                                            uint64_t end_ts,
                                            uint32_t page_len) {
    QueryCursor *q = new QueryCursor(UUIDUtil::generateUUID(),
                                     connectionFeeder,
                                     key,
                                     start_ts,
                                     end_ts,
                                     page_len);
    if(q) {
        //add query to map
        boost::unique_lock<boost::shared_mutex> wmap_loc(map_query_future_mutex);
        map_query_future.insert(make_pair(q->queryID(), q));
    } else {
        releaseQuery(q);
    }
    return q;
}

void IODirectIODriver::releaseQuery(QueryCursor *query_cursor) {
    //acquire write lock
    if(query_cursor == NULL) return;
    boost::unique_lock<boost::shared_mutex> wmap_loc(map_query_future_mutex);
    if(map_query_future.count(query_cursor->queryID())) {
        map_query_future.erase(query_cursor->queryID());
    }
    delete query_cursor;
}
