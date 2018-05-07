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
#include "DefaultPersistenceDriver.h"
#include <chaos/common/healt_system/HealtManager.h>
#include <chaos_metadata_service_client/ChaosMetadataServiceClient.h>

#include <chaos/common/utility/UUIDUtil.h>
#include <chaos/common/network/URL.h>
#include <chaos/common/chaos_constants.h>

#define DPD_LOG_HEAD "[DefaultPersistenceDriver] - "

#define DPD_LAPP LAPP_ << DPD_LOG_HEAD
#define DPD_LDBG LDBG_ << DPD_LOG_HEAD << __PRETTY_FUNCTION__
#define DPD_LERR LERR_ << DPD_LOG_HEAD << __PRETTY_FUNCTION__ << "(" << __LINE__ << ") "
#define MDS_TIMEOUT 5000

using namespace chaos::wan_proxy::persistence;

using namespace chaos::common::data;
using namespace chaos::common::utility;
using namespace chaos::common::network;
using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::channel;
using namespace chaos::common::healt_system;
using namespace chaos::metadata_service_client;

/*---------------------------------------------------------------------------------

 ---------------------------------------------------------------------------------*/
DefaultPersistenceDriver::DefaultPersistenceDriver(NetworkBroker *_network_broker):
    AbstractPersistenceDriver("DefaultPersistenceDriver"),
    network_broker(_network_broker),
    direct_io_client(NULL),
    mds_message_channel(NULL),
    connection_feeder("DefaultPersistenceDriver", this) {


}

/*---------------------------------------------------------------------------------

 ---------------------------------------------------------------------------------*/
DefaultPersistenceDriver::~DefaultPersistenceDriver() {

}

void DefaultPersistenceDriver::init(void *init_data) throw (chaos::CException) {

    //! get the mds message channel
    mds_message_channel = network_broker->getMetadataserverMessageChannel();
    if(!mds_message_channel) throw chaos::CException(-1, "No mds channel found", __PRETTY_FUNCTION__);

    //! get the direct io client
    direct_io_client = network_broker->getSharedDirectIOClientInstance();
    ChaosMetadataServiceClient::getInstance()->init();
    ChaosMetadataServiceClient::getInstance()->start();
    ioLiveDataDriver=ChaosMetadataServiceClient::getInstance()->getDataProxyChannelNewInstance();
    if(!ioLiveDataDriver){
        throw chaos::CException(-1, "No LIVE Channel created", "DefaultPersistenceDriver()");
    }
    CDataWrapper *tmp_data_handler = NULL;

    if(!mds_message_channel->getDataDriverBestConfiguration(&tmp_data_handler, 5000)){
        ChaosUniquePtr<chaos::common::data::CDataWrapper> best_available_da_ptr(tmp_data_handler);
        ioLiveDataDriver->updateConfiguration(best_available_da_ptr.get());
    }
    //InizializableService::initImplementation(direct_io_client,
    //init_data,
    //direct_io_client->getName(),
    // __PRETTY_FUNCTION__);
}

void DefaultPersistenceDriver::deinit() throw (chaos::CException) {

    connection_feeder.clear();

    //if(direct_io_client) {
    //CHAOS_NOT_THROW(InizializableService::deinitImplementation(direct_io_client,
    //														   direct_io_client->getName(),
    //														   __PRETTY_FUNCTION__);)
    //delete(direct_io_client);
    //}

    if(mds_message_channel) network_broker->disposeMessageChannel(mds_message_channel);
}

/*---------------------------------------------------------------------------------

 ---------------------------------------------------------------------------------*/
void DefaultPersistenceDriver::clear() {
    connection_feeder.clear();
}

/*---------------------------------------------------------------------------------

 ---------------------------------------------------------------------------------*/
void DefaultPersistenceDriver::addServerList(const std::vector<std::string>& _cds_address_list) {
    //checkif someone has passed us the device indetification
    DPD_LAPP << "Scan the direction address";

    CDataWrapper *tmp_data_handler = NULL;

    if(!mds_message_channel->getDataDriverBestConfiguration(&tmp_data_handler, 5000)){
        if(tmp_data_handler!=NULL){
            ChaosUniquePtr<chaos::common::data::CDataWrapper> best_available_da_ptr(tmp_data_handler);
            DPD_LDBG <<best_available_da_ptr->getJSONString();
            ChaosUniquePtr<chaos::common::data::CMultiTypeDataArrayWrapper> liveMemAddrConfig(best_available_da_ptr->getVectorValue(DataServiceNodeDefinitionKey::DS_DIRECT_IO_FULL_ADDRESS_LIST));
            if(liveMemAddrConfig.get()){
                size_t numerbOfserverAddressConfigured = liveMemAddrConfig->size();
                for ( int idx = 0; idx < numerbOfserverAddressConfigured; idx++ ){
                    std::string serverDesc = liveMemAddrConfig->getStringElementAtIndex(idx);
                    connection_feeder.addURL(serverDesc);
                }
            }
        }
    }
    //mds_message_channel->ge
    //	connection_feeder.addURL()

    for (std::vector<std::string>::const_iterator it = _cds_address_list.begin();
         it != _cds_address_list.end();
         it++ ){
        if(!common::direct_io::DirectIOClient::checkURL(*it)) {
            DPD_LDBG << "Data proxy server description " << *it << " non well formed";
            continue;
        }
        //add new url to connection feeder
        connection_feeder.addURL(chaos::common::network::URL(*it));
    }

}

/*---------------------------------------------------------------------------------

 ---------------------------------------------------------------------------------*/
void DefaultPersistenceDriver::disposeService(void *service_ptr) {
    if(!service_ptr) return;
    DirectIOChannelsInfo	*info = static_cast<DirectIOChannelsInfo*>(service_ptr);

    if(info->device_client_channel) info->connection->releaseChannelInstance(info->device_client_channel);
    direct_io_client->releaseConnection(info->connection);
    delete(info);
}

/*---------------------------------------------------------------------------------

 ---------------------------------------------------------------------------------*/
void* DefaultPersistenceDriver::serviceForURL(const common::network::URL& url, uint32_t service_index) {
    DPD_LDBG << "Add connection for " << url.getURL();
    DirectIOChannelsInfo * clients_channel = NULL;
    chaos_direct_io::DirectIOClientConnection *tmp_connection = direct_io_client->getNewConnection(url.getURL());
    if(tmp_connection) {
        clients_channel = new DirectIOChannelsInfo();
        clients_channel->connection = tmp_connection;

        //allocate the client channel
        clients_channel->device_client_channel = (DirectIODeviceClientChannel*)tmp_connection->getNewChannelInstance("DirectIODeviceClientChannel");
        if(!clients_channel->device_client_channel) {
            DPD_LDBG << "Error creating client device channel for " << url.getURL();

            //release conenction
            direct_io_client->releaseConnection(tmp_connection);

            //relase struct
            delete(clients_channel);
            return NULL;
        }

        //set this driver instance as event handler for connection
        clients_channel->connection->setEventHandler(this);
        //!put the index in the conenction so we can found it wen we receive event from it
        clients_channel->connection->setCustomStringIdentification(boost::lexical_cast<std::string>(service_index));
    } else {
        DPD_LERR << "Error creating client connection for " << url.getURL();
    }
    return clients_channel;
}

/*---------------------------------------------------------------------------------

 ---------------------------------------------------------------------------------*/
void DefaultPersistenceDriver::handleEvent(DirectIOClientConnection *client_connection,
                                           DirectIOClientConnectionStateType::DirectIOClientConnectionStateType event) {
    //if the channel has bee disconnected turn the relative index offline, if onli reput it online
    boost::unique_lock<boost::shared_mutex>(mutext_feeder);
    uint32_t service_index = boost::lexical_cast<uint32_t>(client_connection->getCustomStringIdentification());
    DEBUG_CODE(DPD_LDBG << "Manage event for service with index " << service_index << " and url " << client_connection->getURL();)
            switch(event) {
        case chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionEventConnected:
            connection_feeder.setURLOnline(service_index);
            break;

        case chaos_direct_io::DirectIOClientConnectionStateType::DirectIOClientConnectionEventDisconnected:
            connection_feeder.setURLOffline(service_index);
            break;
    }
}

// push a dataset
int DefaultPersistenceDriver::pushNewDataset(const std::string& producer_key,
                                             CDataWrapper *new_dataset,
                                             int store_hint) {
    CHAOS_ASSERT(new_dataset)
            int err = 0;
    //ad producer key
    std::map<std::string,cuids_t>::iterator i_cuid=m_cuid.find(producer_key);

    new_dataset->addStringValue(chaos::DataPackCommonKey::DPCK_DEVICE_ID, producer_key);

    new_dataset->addInt32Value(chaos::DataPackCommonKey::DPCK_DATASET_TYPE, chaos::DataPackCommonKey::DPCK_DATASET_TYPE_OUTPUT);
    if(!new_dataset->hasKey(chaos::DataPackCommonKey::DPCK_TIMESTAMP)){
        uint64_t ts = chaos::common::utility::TimingUtil::getTimeStamp();

        // add timestamp of the datapack
        new_dataset->addInt64Value(chaos::DataPackCommonKey::DPCK_TIMESTAMP, ts);
    }
    if(!new_dataset->hasKey(chaos::DataPackCommonKey::DPCK_SEQ_ID)){
        new_dataset->addInt64Value(chaos::DataPackCommonKey::DPCK_SEQ_ID,i_cuid->second.pckid++ );
    }
    if(!new_dataset->hasKey(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_RUN_ID)){
        new_dataset->addInt64Value(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_RUN_ID,i_cuid->second.runid );
    }
    ChaosUniquePtr<SerializationBuffer> serialization(new_dataset->getBSONData());
    //	DPD_LDBG <<" PUSHING:"<<new_dataset->getJSONString();
    DirectIOChannelsInfo	*next_client = static_cast<DirectIOChannelsInfo*>(connection_feeder.getService());
    serialization->disposeOnDelete = !next_client;
    if(next_client) {
        boost::shared_lock<boost::shared_mutex>(next_client->connection_mutex);

        //free the packet
        serialization->disposeOnDelete = false;
        if((err =(int)next_client->device_client_channel->storeAndCacheDataOutputChannel(producer_key+ chaos::DataPackPrefixID::OUTPUT_DATASET_POSTFIX,
                                                                                         (void*)serialization->getBufferPtr(),
                                                                                         (uint32_t)serialization->getBufferLen(),
                                                                                         (chaos::DataServiceNodeDefinitionType::DSStorageType)store_hint))) {
            DPD_LERR << "Error storing dataset with code:" << err;
        }
    } else {
        DEBUG_CODE(DPD_LDBG << "No available socket->loose packet");
        err = -1;
    }

    return err;
}

// get a dataset
int DefaultPersistenceDriver::getLastDataset(const std::string& producer_key,
                                             chaos::common::data::CDataWrapper **last_dataset) {
    int err = 0;
    uint32_t size = 0;
    char* result = NULL;
    DirectIOChannelsInfo	*next_client = static_cast<DirectIOChannelsInfo*>(connection_feeder.getService());
    if(!next_client) return err;

    boost::shared_lock<boost::shared_mutex>(next_client->connection_mutex);
    next_client->device_client_channel->requestLastOutputData(producer_key, (void**)&result, size);
    *last_dataset = new CDataWrapper(result);
    return err;
}

//! register the dataset of ap roducer
int DefaultPersistenceDriver::registerDataset(const std::string& producer_key,
                                              chaos::common::data::CDataWrapper& last_dataset) {
    CHAOS_ASSERT(mds_message_channel);
    int ret;

    last_dataset.addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, producer_key);
    last_dataset.addStringValue(chaos::NodeDefinitionKey::NODE_RPC_DOMAIN, chaos::common::utility::UUIDUtil::generateUUIDLite());
    last_dataset.addStringValue(chaos::NodeDefinitionKey::NODE_RPC_ADDR, network_broker->getRPCUrl());
    last_dataset.addStringValue("mds_control_key","none");
    if((ret=mds_message_channel->sendNodeRegistration(last_dataset, true, 10000)) ==0){
        CDataWrapper mdsPack;
        mdsPack.addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, producer_key);
        mdsPack.addStringValue(chaos::NodeDefinitionKey::NODE_TYPE, chaos::NodeType::NODE_TYPE_CONTROL_UNIT);
        ret = mds_message_channel->sendNodeLoadCompletion(mdsPack, true, 10000);
        HealtManager::getInstance()->addNewNode(producer_key);
        HealtManager::getInstance()->addNodeMetric(producer_key,
                                                   chaos::ControlUnitHealtDefinitionValue::CU_HEALT_OUTPUT_DATASET_PUSH_RATE,
                                                   chaos::DataType::TYPE_DOUBLE);
        HealtManager::getInstance()->addNodeMetricValue(producer_key,
                                                        NodeHealtDefinitionKey::NODE_HEALT_STATUS,
                                                        NodeHealtDefinitionValue::NODE_HEALT_STATUS_START,
                                                        true);
        chaos::common::async_central::AsyncCentralManager::getInstance()->addTimer(this, chaos::common::constants::CUTimersTimeoutinMSec, chaos::common::constants::CUTimersTimeoutinMSec);
        std::map<std::string,cuids_t>::iterator i_cuid=m_cuid.find(producer_key);
        if(i_cuid==m_cuid.end()){
            cuids_t tt;
            DEBUG_CODE(DPD_LDBG << "Adding new device:"<<producer_key);
            tt.pckid=0;
            tt.runid=chaos::common::utility::TimingUtil::getTimeStamp();
            tt.last_pckid=0;
            m_cuid[producer_key]=tt;
        }
    }

    return ret;
}
void DefaultPersistenceDriver::timeout(){
    uint64_t rate_acq_ts = TimingUtil::getTimeStamp();
    for(std::map<std::string,cuids_t>::iterator i_cuid=m_cuid.begin();i_cuid!=m_cuid.end();i_cuid++){

        double time_offset = (double(rate_acq_ts - i_cuid->second.last_ts))/1000.0; //time in seconds
        double output_ds_rate = (time_offset>0)?( i_cuid->second.pckid-i_cuid->second.last_pckid)/time_offset:0; //rate in seconds
        HealtManager::getInstance()->addNodeMetricValue(i_cuid->first,
                                                        chaos::ControlUnitHealtDefinitionValue::CU_HEALT_OUTPUT_DATASET_PUSH_RATE,
                                                        output_ds_rate,true);
        i_cuid->second.last_ts=rate_acq_ts;
        i_cuid->second.last_pckid=i_cuid->second.pckid;

    }

}
void DefaultPersistenceDriver::searchMetrics(const std::string&search_string,ChaosStringVector& metrics,bool alive){
    int node_type=2; //CU
    ChaosStringVector node_tmp;
    metrics.clear();
    if(mds_message_channel->searchNode(search_string,
                                       node_type,
                                       alive,
                                       0,
                                       10000,
                                       node_tmp,
                                       5000)==0){
        for(ChaosStringVector::iterator i=node_tmp.begin();node_tmp.end()!=i;i++){
            size_t value_len;
            const int dt[]={
                DataPackCommonKey::DPCK_DATASET_TYPE_OUTPUT,
                DataPackCommonKey::DPCK_DATASET_TYPE_INPUT};
            for(int cnt=0;cnt<sizeof(dt)/sizeof(int);cnt++){
                std::string lkey=*i+chaos::datasetTypeToPostfix(dt[cnt]);
                char *value = ioLiveDataDriver->retriveRawData(lkey,(size_t*)&value_len);
                if(value){
                    chaos::common::data::CDataWrapper *tmp = new CDataWrapper(value);
                    ChaosStringVector ds;
                    tmp->getAllKey(ds);
                    for(ChaosStringVector::iterator ii=ds.begin();ds.end()!=ii;ii++){
                        std::string metric=*i+"/"+chaos::datasetTypeToHuman(dt[cnt])+"/"+*ii;
                        metrics.push_back(metric);
                    }
            }
            }

        }
    }
}

chaos::common::data::CDWShrdPtr DefaultPersistenceDriver::searchMetrics(const std::string&search_string,bool alive){
    chaos::common::data::CDWShrdPtr ret(new CDataWrapper());
    int node_type=2; //CU
    ChaosStringVector node_tmp;
    if(mds_message_channel->searchNode(search_string,
                                       node_type,
                                       alive,
                                       0,
                                       10000,
                                       node_tmp,
                                       5000)==0){
        for(ChaosStringVector::iterator i=node_tmp.begin();node_tmp.end()!=i;i++){


            CDataWrapper* out=0;
            DEBUG_CODE(DPD_LDBG << "finding description of:"<<*i);

            if (mds_message_channel->getFullNodeDescription(*i, &out, MDS_TIMEOUT) == 0) {

                if(out && (out->hasKey(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION))&&(out->isCDataWrapperValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION))){
                    chaos::common::data::CDWUniquePtr ds(out->getCSDataValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION));
                    chaos::common::data::CMultiTypeDataArrayWrapper*w =ds->getVectorValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION);
                    for(int idx=0;idx<w->size();idx++){
                        chaos::common::data::CDWUniquePtr ws(w->getCDataWrapperElementAtIndex(idx));
                        ret->appendCDataWrapperToArray(*(ws.get()));
                    }
                    ret->finalizeArrayForKey(*i);
                    if(w){
                        delete w;
                    }
                    //          DEBUG_CODE(DPD_LDBG << "adding:"<<ret->getCompliantJSONString());

                    /*if(ds->hasKey(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION)&&ds->isVector(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION) ){
                        ChaosUniquePtr<CMultiTypeDataArrayWrapper> dw(ds->getVectorValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION));
                        for (int idx = 0; idx < dw->size(); idx++) {

                        }
                    }*/

                }
                if(out){
                    delete out;
                }
            }
        }

    }
    return ret;
}

int DefaultPersistenceDriver::queryMetrics(const std::string& start,const std::string& end,const std::vector<std::string>& metrics_name,metrics_results_t& res,int limit){
    int ret=0;
    boost::regex expr("(.*)/(.*)$");
    uint64_t start_t=chaos::common::utility::TimingUtil::getTimestampFromString(start,true);
    uint64_t end_t=chaos::common::utility::TimingUtil::getTimestampFromString(end,true);
    std::map<std::string,std::vector<std::string> > accesses;
    for ( int index = 0; index < metrics_name.size(); ++index ){
        std::string tname=metrics_name[index];
        DPD_LDBG << "Target:"<<tname;

        boost::cmatch what;
        if(regex_match(tname.c_str(), what, expr)){
            std::map<std::string,std::vector<std::string> >::iterator i=accesses.find(what[1]);
            if(i!=accesses.end()){
                DPD_LDBG << " variable:"<<what[2]<< " adding to access:"<<i->first;

                i->second.push_back(what[2]);
            } else {
                DPD_LDBG << " variable:"<<what[2]<< " to new access:"<<what[1];
                accesses[what[1]].push_back(what[2]);
            }
        }
    }

    // perform queries
    for(std::map<std::string,std::vector<std::string> >::iterator i=accesses.begin();i!=accesses.end();i++){
        boost::cmatch what;

        if(regex_match(i->first.c_str(), what, expr)){
            std::string cuname=what[1];
            std::string dir=what[2];
            DPD_LDBG << " access CU:"<<cuname<<" channel:"<<dir<< " # vars:"<<i->second.size();
            int type =HumanTodatasetType(dir);
            std::string dst=cuname+chaos::datasetTypeToPostfix(type);
            DPD_LDBG << " perform query to:"<<dst<<" start:"<<start_t<<" end:"<<end_t<<" limit:"<<limit;

            chaos::common::io::QueryCursor *pnt=ioLiveDataDriver->performQuery(dst,start_t,end_t,100);
            if(pnt ){
                int cnt=0;
                while(pnt->hasNext()&& cnt++<limit){
                    chaos::common::data::CDWShrdPtr ds(pnt->next());
                    uint64_t ts=0;
                    if(ds->hasKey(chaos::DataPackCommonKey::DPCK_TIMESTAMP)){
                        ts=ds->getInt64Value(chaos::DataPackCommonKey::DPCK_TIMESTAMP);
                    }
                    // get all other variables.. if any
                    for(std::vector<std::string>::iterator j=i->second.begin();j!=i->second.end();j++){
                        if(ds->hasKey(*j)){
                            metric_t tmp;
                            tmp.milli_ts=ts;
                            std::string metric_name=i->first+"/"+*j;
                            if(ds->isVector(*j)){
                                chaos::common::data::CMultiTypeDataArrayWrapper*w =ds->getVectorValue(*j);
                                for(int cnt=0;cnt<w->size();cnt++){
                                    tmp.idx=cnt;
                                    if(w->isDoubleElementAtIndex(cnt)){
                                        tmp.value=w->getDoubleElementAtIndex(cnt);
                                        res[metric_name].push_back(tmp);
                                    }
                                    if(w->isInt32ElementAtIndex(cnt)){
                                        tmp.value=w->getInt32ElementAtIndex(cnt);
                                        res[metric_name].push_back(tmp);
                                    }
                                    if(w->isInt64ElementAtIndex(cnt)){
                                        tmp.value=w->getInt64ElementAtIndex(cnt);
                                        res[metric_name].push_back(tmp);
                                    }
                                }
                            } else {
                                tmp.idx=0;
                                if(ds->isDoubleValue(*j)){
                                    tmp.value=ds->getDoubleValue(*j);
                                    res[metric_name].push_back(tmp);

                                }
                                if(ds->isInt32Value(*j)){
                                    tmp.value=ds->getInt32Value(*j);
                                    res[metric_name].push_back(tmp);
                                }
                                if(ds->isInt64Value(*j)){
                                    tmp.value=ds->getInt64Value(*j);
                                    res[metric_name].push_back(tmp);
                                }
                            }
                        }
                    }
                }
                ioLiveDataDriver->releaseQuery(pnt);
            }
        }
    }

    return ret;
}
