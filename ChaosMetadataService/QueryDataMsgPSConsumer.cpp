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
#include "QueryDataMsgPSConsumer.h"
#include "api/logging/SubmitEntryBase.h"
#include "api/node/NodeSearch.h"
using namespace chaos::metadata_service::worker;
using namespace chaos::metadata_service::cache_system;

using namespace chaos::metadata_service::persistence;
using namespace chaos::metadata_service::persistence::data_access;

using namespace chaos::common::data::structured;
using namespace chaos::common::utility;
using namespace chaos::common::network;
using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::channel;

#define INFO INFO_LOG(QueryDataMsgPSConsumer)
#define DBG DBG_LOG(QueryDataMsgPSConsumer)
#define ERR ERR_LOG(QueryDataMsgPSConsumer)

#define MAX_LOG_QUEUE 100
//constructor
namespace chaos {
namespace metadata_service {

/*QueryDataMsgPSConsumer::QueryDataMsgPSConsumer(){
 }

QueryDataMsgPSConsumer::~QueryDataMsgPSConsumer() {
}
*/
std::map<std::string, uint64_t>  QueryDataMsgPSConsumer::alive_map;
boost::mutex QueryDataMsgPSConsumer::map_m;
QueryDataMsgPSConsumer::QueryDataMsgPSConsumer(const std::string& id)
    : groupid(id) {
  if (GlobalConfiguration::getInstance()->getConfiguration()->hasKey(InitOption::OPT_HA_ZONE_NAME)) {
    groupid = groupid + "_" + GlobalConfiguration::getInstance()->getConfiguration()->getStringValue(InitOption::OPT_HA_ZONE_NAME);
  }

  msgbrokerdrv = GlobalConfiguration::getInstance()->getOption<std::string>(InitOption::OPT_MSG_BROKER_DRIVER);

  cons = chaos::common::message::MessagePSDriver::getConsumerDriver(msgbrokerdrv, groupid);
}
void QueryDataMsgPSConsumer::messageHandler(const chaos::common::message::ele_t& data) {
  try {
  ChaosStringSetConstSPtr meta_tag_set;


  if (data.cd->hasKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_TAG)) {
    ChaosStringSet* tag = new ChaosStringSet();
    tag->insert(data.cd->getStringValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_TAG));
    meta_tag_set.reset(tag);
  }
  std::string kp ;//= data.key;

  //std::replace(kp.begin(), kp.end(), '.', '/');
  //DBG<<"data from:"<<kp<<" size:"<<data.cd->getBSONRawSize();
  if(data.cd->hasKey(DataPackCommonKey::DPCK_DATASET_TYPE)&&data.cd->hasKey(NodeDefinitionKey::NODE_UNIQUE_ID)){
    uint64_t now = TimingUtil::getTimeStamp();

    int pktype=data.cd->getInt32Value(DataPackCommonKey::DPCK_DATASET_TYPE);
    int64_t ts=0;
    uint32_t                st=(uint32_t)DataServiceNodeDefinitionType::DSStorageTypeLive;
    if(data.cd->hasKey(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE)){
      st=data.cd->getInt32Value(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE);
      if(pktype!=DataPackCommonKey::DPCK_DATASET_TYPE_OUTPUT){
          st|=(uint32_t)DataServiceNodeDefinitionType::DSStorageTypeLive;
      }
    }
    kp=data.cd->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID)+datasetTypeToPostfix(pktype);
    int32_t lat=0;
    if(pktype==DataPackCommonKey::DPCK_DATASET_TYPE_LOG){
        if(data.cd->hasKey(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP)){
          ts=data.cd->getInt64Value(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP);
          lat=now-ts;
          if(lat>SKIP_OLDER_THAN){
              ERR<<kp<<" log too old: "<<lat<< " ms, skipping...";
              return ;
          }
          data.cd->addInt32Value(DataPackCommonKey::NODE_MDS_TIMEDIFF,lat );

        }
    //  DBG<<"Queue:"<<CObjectProcessingPriorityQueue<CDataWrapper>::queueSize()<<" LOG:"<<data.cd->getJSONString();
        if(CObjectProcessingPriorityQueue<CDataWrapper>::queueSize()<MAX_LOG_QUEUE){
          CObjectProcessingPriorityQueue<CDataWrapper>::push(data.cd,0);
        } else {
          ERR<<kp<<"] too many logs on queue for DB:"<<CObjectProcessingPriorityQueue<CDataWrapper>::queueSize();
          return;
        }
    } else if(data.cd->hasKey(DataPackCommonKey::DPCK_TIMESTAMP)){
        ts=data.cd->getInt64Value(DataPackCommonKey::DPCK_TIMESTAMP);
        lat=(now-ts);
        data.cd->addInt32Value(DataPackCommonKey::NODE_MDS_TIMEDIFF,lat );
        if((pktype==DataPackCommonKey::DPCK_DATASET_TYPE_HEALTH)){
          if(lat>(chaos::common::constants::HBTimersTimeoutinMSec*2)){
         // health too old
            return;
          }
        } else if((pktype==DataPackCommonKey::DPCK_DATASET_TYPE_OUTPUT)||(pktype==DataPackCommonKey::DPCK_DATASET_TYPE_INPUT)){
              if(((st==0) || (st==DataServiceNodeDefinitionType::DSStorageTypeLive))){
              if(lat>SKIP_OLDER_THAN){
                 ERR<<kp<<" too old: "<<lat<< " ms, skipping...";
              // output too old
                return;
              }
          data.cd->removeKey(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE);
          data.cd->removeKey(DataPackCommonKey::DPCK_DATASET_TYPE);
          
        } 
       
   
      }
    }
      
    QueryDataConsumer::consumePutEvent(kp, (uint8_t)st, meta_tag_set, *(data.cd.get()));
  }
  } catch(const chaos::CException& e ){
    ERR<<"Chaos Exception caught processing key:"<<data.key<<" ("<<data.off<<","<<data.par<<") error:"<<e.what();
  } catch(...){
    ERR<<"Unknown Exception caught processing key:"<<data.key<<" ("<<data.off<<","<<data.par<<")";

  }
}
  void QueryDataMsgPSConsumer::processBufferElement(chaos::common::data::CDWShrdPtr log_entry){
      chaos::metadata_service::api::logging::SubmitEntryBase se;
  //      DBG<<"Queue:"<<CObjectProcessingPriorityQueue<CDataWrapper>::queueSize()<<" WRITE ";

      se.execute(log_entry->clone());        
      
  }

void QueryDataMsgPSConsumer::messageError(const chaos::common::message::ele_t& data) {
  ChaosStringSetConstSPtr meta_tag_set;
      boost::mutex::scoped_lock ll(map_m);
    std::string path=data.key;
    std::replace(path.begin(), path.end(), '.', '/');

  //  std::map<std::string, uint64_t>::iterator i=alive_map.find(path);
    if(data.cd.get()&&data.cd->hasKey("msg")&&data.cd->hasKey("err")){
      ERR<<"key:"<<data.key<<" ["<<path<<"] err msg:"<<data.cd->getStringValue("msg")<<" err:"<<data.cd->getInt32Value("err");
    }
  /*  if(i!=alive_map.end()){
      DBG<<" removing from alive list:"<<i->first;
      alive_map.erase(i);
    } else {
      DBG<<path<<" is not in the alive list";

    }*/

}

void QueryDataMsgPSConsumer::init(void* init_data) {

  QueryDataConsumer::init(init_data);
  msgbroker = GlobalConfiguration::getInstance()->getOption<std::string>(InitOption::OPT_MSG_BROKER_SERVER);
  DBG<<"Initialize, my broker is:"<<msgbroker;

  CObjectProcessingPriorityQueue<CDataWrapper>::init(1);

  cons->addServer(msgbroker);

  cons->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONARRIVE, boost::bind(&QueryDataMsgPSConsumer::messageHandler, this, _1));
  cons->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONERROR, boost::bind(&QueryDataMsgPSConsumer::messageError, this, _1));

/*
  if (cons->setOption("auto.offset.reset", "earliest") != 0) {
    throw chaos::CException(-1, "cannot set offset:" + cons->getLastError(), __PRETTY_FUNCTION__);
  }
  if (cons->setOption("topic.metadata.refresh.interval.ms", "5000") != 0) {
    throw chaos::CException(-1, "cannot set refresh topic:" + cons->getLastError(), __PRETTY_FUNCTION__);
  }
  */
  if (cons->applyConfiguration() != 0) {
    throw chaos::CException(-1, "cannot initialize Publish Subscribe:" + cons->getLastError(), __PRETTY_FUNCTION__);
  }
}
void QueryDataMsgPSConsumer::subscribeProcess(int attempt){
DBG << "Starting SubscribeProcess";

api::node::NodeSearch node;
sleep(10);

while(attempt--){
  std::vector<std::string> nodes=node.search("",(chaos::NodeType::NodeSearchType)(((int)chaos::NodeType::node_type_ceu )| ((int)chaos::NodeType::node_type_agent)| ((int)chaos::NodeType::node_type_us))); // search CEU

  DBG <<"] Found " << nodes.size()<< " to subscribe";

  for(std::vector<std::string>::iterator i=nodes.begin();i!=nodes.end();i++){
    if(i->size()){
      DBG <<"] Subscribing to:" << *i;

      if (cons->subscribe(*i) != 0) {
          ERR <<" cannot subscribe to :" << *i<<" err:"<<cons->getLastError();
                  
      } else {
          DBG <<"] Subscribed to:" << *i;
          
      }
    }  
  }
}
} 
void QueryDataMsgPSConsumer::start() {
  DBG << "Starting Msg consumer";

  cons->start();
  boost::thread(&QueryDataMsgPSConsumer::subscribeProcess, this,1);

 /* std::string keysub="CHAOS_LOG";
  if (cons->subscribe(keysub) != 0) {
      ERR <<" cannot subscribe to :" << keysub<<" err:"<<cons->getLastError();
              
  }*/ 
}

void QueryDataMsgPSConsumer::stop() {
  DBG << "Stopping Msg consumer";
  cons->stop();
}

void QueryDataMsgPSConsumer::deinit() {
  QueryDataConsumer::deinit();
  DBG << "Wait for queue will empty";
  CObjectProcessingPriorityQueue<CDataWrapper>::deinit(true);
  DBG << "Queue is empty";
}

int QueryDataMsgPSConsumer::consumeHealthDataEvent(const std::string&            key,
                                                   const uint8_t                 hst_tag,
                                                   const ChaosStringSetConstSPtr meta_tag_set,
                                                   BufferSPtr                    channel_data) {
  int err = 0;
  
    boost::mutex::scoped_lock ll(map_m);
    
    
/*    bool isACUEU=(channel_data.get()==NULL)||(health_data_pack.hasKey(chaos::ControlUnitHealtDefinitionValue::CU_HEALT_OUTPUT_DATASET_PUSH_RATE));

    if(isACUEU){
      DBG << "Received healt from:"<<key<<" is:"<<((channel_data.get()==NULL)?"registration":"normal");

      std::string rootname = key;
      size_t      pos      = key.find(NodeHealtDefinitionKey::HEALT_KEY_POSTFIX);
      if (pos != std::string::npos) {
        rootname.erase(pos, strlen(NodeHealtDefinitionKey::HEALT_KEY_POSTFIX));
      }
      if(alive_map.find(rootname)==alive_map.end()){
        if (cons->subscribe(rootname) != 0) {
              ERR <<"] cannot subscribe to :" << rootname<<" err:"<<cons->getLastError();
              
            } else {
              alive_map[rootname]= TimingUtil::getTimeStamp();

              DBG <<"] Subscribed to:" << rootname<<" at:"<<alive_map[rootname];
            }
      }

      
    }
  }*/
  if(channel_data.get()==NULL || channel_data->data()==NULL){
    DBG<<"Empty health for:\""<<key<<"\" registration pack";
    if(alive_map.find(key)==alive_map.end()){
        if (cons->subscribe(key) != 0) {
              ERR <<"] cannot subscribe to :" << key<<" err:"<<cons->getLastError();
              
            } else {
              alive_map[key]= TimingUtil::getTimeStamp();

              DBG <<"] Subscribed to:" << key<<" at:"<<alive_map[key];
            }
      }
    return 0;
  }
  CDataWrapper health_data_pack((char *)channel_data->data());

  return QueryDataConsumer::consumeHealthDataEvent(key, hst_tag, meta_tag_set, channel_data);
}

int QueryDataMsgPSConsumer::consumeGetEvent(chaos::common::data::BufferSPtr                             key_data,
                                            uint32_t                                                    key_len,
                                            opcode_headers::DirectIODeviceChannelHeaderGetOpcodeResult& result_header,
                                            chaos::common::data::BufferSPtr&                            result_value) {
  return QueryDataConsumer::consumeGetEvent(key_data, key_len, result_header, result_value);
}

int QueryDataMsgPSConsumer::consumeGetEvent(opcode_headers::DirectIODeviceChannelHeaderMultiGetOpcode&       header,
                                            const ChaosStringVector&                                         keys,
                                            opcode_headers::DirectIODeviceChannelHeaderMultiGetOpcodeResult& result_header,
                                            chaos::common::data::BufferSPtr&                                 result_value,
                                            uint32_t&                                                        result_value_len) {
  return QueryDataConsumer::consumeGetEvent(header, keys, result_header, result_value, result_value_len);
}

int QueryDataMsgPSConsumer::consumeDataCloudQuery(opcode_headers::DirectIODeviceChannelHeaderOpcodeQueryDataCloud& query_header,
                                                  const std::string&                                               search_key,
                                                  const ChaosStringSet&                                            meta_tags,
                                                  const ChaosStringSet&                                            projection_keys,
                                                  const uint64_t                                                   search_start_ts,
                                                  const uint64_t                                                   search_end_ts,
                                                  opcode_headers::SearchSequence&                                  last_element_found_seq,
                                                  opcode_headers::QueryResultPage&                                 page_element_found) {
  return QueryDataConsumer::consumeDataCloudQuery(query_header,
                                                  search_key,
                                                  meta_tags,
                                                  projection_keys,
                                                  search_start_ts,
                                                  search_end_ts,
                                                  last_element_found_seq,
                                                  page_element_found);
}

int QueryDataMsgPSConsumer::consumeDataIndexCloudQuery(opcode_headers::DirectIODeviceChannelHeaderOpcodeQueryDataCloud& query_header,
                                                       const std::string&                                               search_key,
                                                       const ChaosStringSet&                                            meta_tags,
                                                       const ChaosStringSet&                                            projection_keys,
                                                       const uint64_t                                                   search_start_ts,
                                                       const uint64_t                                                   search_end_ts,
                                                       opcode_headers::SearchSequence&                                  last_element_found_seq,
                                                       opcode_headers::QueryResultPage&                                 page_element_found) {
  return QueryDataConsumer::consumeDataIndexCloudQuery(query_header,
                                                       search_key,
                                                       meta_tags,
                                                       projection_keys,
                                                       search_start_ts,
                                                       search_end_ts,
                                                       last_element_found_seq,
                                                       page_element_found);
}

int QueryDataMsgPSConsumer::consumeDataCloudDelete(const std::string& search_key,
                                                   uint64_t           start_ts,
                                                   uint64_t           end_ts) {
  return QueryDataConsumer::consumeDataCloudDelete(search_key,
                                                   start_ts,
                                                   end_ts);
}

int QueryDataMsgPSConsumer::countDataCloud(const std::string& search_key,
                                           uint64_t           start_ts,
                                           uint64_t           end_ts,
                                           uint64_t&          count) {
  return QueryDataConsumer::countDataCloud(search_key,
                                           start_ts,
                                           end_ts,
                                           count);
}

//---------------- DirectIOSystemAPIServerChannelHandler -----------------------
int QueryDataMsgPSConsumer::consumeGetDatasetSnapshotEvent(opcode_headers::DirectIOSystemAPIChannelOpcodeNDGSnapshotHeader& header,
                                                           const std::string&                                               producer_id,
                                                           chaos::common::data::BufferSPtr&                                 channel_found_data,
                                                           DirectIOSystemAPISnapshotResultHeader&                           result_header) {
  return QueryDataConsumer::consumeGetDatasetSnapshotEvent(header,
                                                           producer_id,
                                                           channel_found_data,
                                                           result_header);
}

int QueryDataMsgPSConsumer::consumeLogEntries(const std::string&       node_name,
                                              const ChaosStringVector& log_entries) {
  return QueryDataConsumer::consumeLogEntries(node_name, log_entries);
}

/*
int QueryDataMsgPSConsumer::consumeDataCloudQuery(DirectIODeviceChannelHeaderOpcodeQueryDataCloud& query_header,
                                             const std::string& search_key,
                                             const ChaosStringSet& meta_tags,
                                             const ChaosStringSet& projection_keys,
                                             const uint64_t search_start_ts,
                                             const uint64_t search_end_ts,
                                             SearchSequence& last_element_found_seq,
                                             QueryResultPage& page_element_found) {
    
    int err = 0;
    //execute the query
    ObjectStorageDataAccess *obj_storage_da = DriverPoolManager::getInstance()->getObjectStorageDrv().getDataAccess<object_storage::abstraction::ObjectStorageDataAccess>();
    if((err = obj_storage_da->findObject(search_key,
                                         meta_tags,
                                         projection_keys,
                                         search_start_ts,
                                         search_end_ts,
                                         query_header.field.record_for_page,
                                         page_element_found,
                                         last_element_found_seq))) {
        ERR << CHAOS_FORMAT("Error performing cloud query with code %1%", %err);
    }
    return err;
}


int QueryDataMsgPSConsumer::consumeDataIndexCloudQuery(opcode_headers::DirectIODeviceChannelHeaderOpcodeQueryDataCloud& query_header,
                                                  const std::string& search_key,
                                                  const ChaosStringSet& meta_tags,
                                                  const ChaosStringSet& projection_keys,
                                                  const uint64_t search_start_ts,
                                                  const uint64_t search_end_ts,
                                                  opcode_headers::SearchSequence& last_element_found_seq,
                                                  opcode_headers::QueryResultPage& page_element_found) {
    int err = 0;
    //execute the query
    DataSearch search;
    search.key = search_key;
    search.meta_tags = meta_tags;
    search.projection_keys = projection_keys;
    search.timestamp_from = search_start_ts;
    search.timestamp_to = search_end_ts;
    search.page_len = query_header.field.record_for_page;
    ObjectStorageDataAccess *obj_storage_da = DriverPoolManager::getInstance()->getObjectStorageDrv().getDataAccess<object_storage::abstraction::ObjectStorageDataAccess>();
    if((err = obj_storage_da->findObjectIndex(search,
                                              page_element_found,
                                              last_element_found_seq))) {
        ERR << CHAOS_FORMAT("Error performing cloud query with code %1%", %err);
    }
    return err;
}

int QueryDataMsgPSConsumer::consumeGetEvent(chaos::common::data::BufferSPtr key_data,
                                       uint32_t key_len,
                                       opcode_headers::DirectIODeviceChannelHeaderGetOpcodeResult& result_header,
                                       chaos::common::data::BufferSPtr& result_value) {
    int err = 0;
    std::string key(key_data->data(),
                    key_data->size());
    //debug check
    //protected access to cached driver
    CacheDriver& cache_slot = DriverPoolManager::getInstance()->getCacheDrv();
    
    //get data
    err = cache_slot.getData(key,
                             result_value);
    if((err == 0 )&&
       result_value &&
       result_value->size()) {
        result_header.value_len = (uint32_t)result_value->size();
    }
    return err;
}

int QueryDataMsgPSConsumer::consumeGetEvent(opcode_headers::DirectIODeviceChannelHeaderMultiGetOpcode& header,
                                       const ChaosStringVector& keys,
                                       opcode_headers::DirectIODeviceChannelHeaderMultiGetOpcodeResult& result_header,
                                       BufferSPtr& result_value,
                                       uint32_t& result_value_len) {
    int err = 0;
    //debug check
    //protected access to cached driver
    CacheDriver& cache_slot = DriverPoolManager::getInstance()->getCacheDrv();
    try{
        //get data
        DataBuffer data_buffer;
        MultiCacheData multi_cached_data;
        err = cache_slot.getData(keys,
                                 multi_cached_data);
        for(ChaosStringVectorConstIterator it = keys.begin(),
            end = keys.end();
            it != end;
            it++) {
            const CacheData& cached_element = multi_cached_data[*it];
            if(!cached_element ||
               cached_element->size() == 0) {
                //element has not been found so we need to create and empty cdata wrapper
                CDataWrapper tmp;
                int size = 0;
                const char * d_ptr = tmp.getBSONRawData(size);
                data_buffer.writeByte(d_ptr,
                                      size);
            } else {
                data_buffer.writeByte(cached_element->data(),
                                      (int32_t)cached_element->size());
            }
        }
        
        result_header.number_of_result = (uint32_t)multi_cached_data.size();
        result_value_len = data_buffer.getCursorLocation();
        result_value = ChaosMakeSharedPtr<Buffer>(data_buffer.release(), result_value_len, result_value_len, true);
        
    } catch(...) {}
    return err;
}

int QueryDataMsgPSConsumer::getDataByIndex(const chaos::common::data::VectorCDWShrdPtr& indexes,
                                      chaos::common::data::VectorCDWShrdPtr& found_data) {
    
    ObjectStorageDataAccess *obj_storage_da = DriverPoolManager::getInstance()->getObjectStorageDrv().getDataAccess<object_storage::abstraction::ObjectStorageDataAccess>();
    std::for_each(indexes.begin(), indexes.end(), [&obj_storage_da, &found_data](const CDWShrdPtr& index){
        int err = 0;
        CDWShrdPtr data;
        if((err = obj_storage_da->getObjectByIndex(index, data)) == 0) {
            found_data.push_back(data);
        } else {
            ERR << CHAOS_FORMAT("Error fetching data using index(%1%) with code %2%", %data->getJSONString()%err);
        }
    });
    return 0;
}

int QueryDataMsgPSConsumer::consumeDataCloudDelete(const std::string& search_key,
                                              uint64_t start_ts,
                                              uint64_t end_ts){
    int err = 0;
    VectorObject reuslt_object_found;
    ObjectStorageDataAccess *obj_storage_da = DriverPoolManager::getInstance()->getObjectStorageDrv().getDataAccess<object_storage::abstraction::ObjectStorageDataAccess>();
    if((err = obj_storage_da->deleteObject(search_key,
                                           start_ts,
                                           end_ts))) {
        ERR << CHAOS_FORMAT("Error performing cloud query with code %1%", %err);
    }
    return err;
}
int QueryDataMsgPSConsumer::countDataCloud(const std::string& search_key,
                                       uint64_t start_ts,
                                       uint64_t end_ts,
                                       uint64_t& count){
    int err = 0;
    ObjectStorageDataAccess *obj_storage_da = DriverPoolManager::getInstance()->getObjectStorageDrv().getDataAccess<object_storage::abstraction::ObjectStorageDataAccess>();
    if((err = obj_storage_da->countObject(search_key,
                                           start_ts,
                                           end_ts,count))) {
        ERR << CHAOS_FORMAT("Error performing count cloud query with code %1%", %err);
    }
    return err;

}
            
#pragma mark DirectIOSystemAPIServerChannelHandler
// Return the dataset for a producerkey ona specific snapshot
int QueryDataMsgPSConsumer::consumeGetDatasetSnapshotEvent(opcode_headers::DirectIOSystemAPIChannelOpcodeNDGSnapshotHeader& header,
                                                      const std::string& producer_id,
                                                      chaos::common::data::BufferSPtr& channel_found_data,
                                                      DirectIOSystemAPISnapshotResultHeader &result_header) {
    int err = 0;
    std::string channel_type;
    //CHAOS_ASSERT(api_result)
    SnapshotDataAccess *s_da = DriverPoolManager::getInstance()->getPersistenceDataAccess<SnapshotDataAccess>();
    
    //trduce int to postfix channel type
    switch(header.field.channel_type) {
        case 0:
            channel_type = DataPackPrefixID::OUTPUT_DATASET_POSTFIX;
            break;
        case 1:
            channel_type = DataPackPrefixID::INPUT_DATASET_POSTFIX;
            break;
        case 2:
            channel_type = DataPackPrefixID::CUSTOM_DATASET_POSTFIX;
            break;
        case 3:
            channel_type = DataPackPrefixID::SYSTEM_DATASET_POSTFIX;
            break;
            
    }
    
    if((err = s_da->snapshotGetDatasetForProducerKey(header.field.snap_name,
                                                     producer_id,
                                                     channel_type,
                                                     channel_found_data))) {
        std::strcpy(result_header.error_message, "Error retriving the snapshoted dataaset for producer key");
        ERR << result_header.error_message << "[" << header.field.snap_name << "/" << producer_id<<"]";
    } else {
        if(channel_found_data &&
           channel_found_data->size()) {
            result_header.error = 0;
            std::strcpy(result_header.error_message, "Snapshot found");
        } else {
            result_header.error = -2;
            std::strcpy(result_header.error_message, "Channel data not found in snapshot");
            
        }
    }
    return err;
}

int QueryDataMsgPSConsumer::consumeLogEntries(const std::string& node_name,
                                         const ChaosStringVector& log_entries) {
    int err = 0;
    for(ChaosStringVectorConstIterator it = log_entries.begin(),
        end = log_entries.end();
        it != end;
        it++) {
        AgentDataAccess *a_da = DriverPoolManager::getInstance()->getPersistenceDataAccess<AgentDataAccess>();
        if((err = a_da->pushLogEntry(node_name, *it))){
            ERR << CHAOS_FORMAT("Error push entry for node %1%", %node_name);
        }
    }
    return err;
}
*/
}  // namespace metadata_service
}  // namespace chaos