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
std::map<std::string, uint64_t> QueryDataMsgPSConsumer::alive_map;
boost::mutex                    QueryDataMsgPSConsumer::map_m;
QueryDataMsgPSConsumer::QueryDataMsgPSConsumer(const std::string& id)
    : groupid(id) {
  if (GlobalConfiguration::getInstance()->getConfiguration()->hasKey(InitOption::OPT_HA_ZONE_NAME)) {
    groupid = groupid + "_" + GlobalConfiguration::getInstance()->getConfiguration()->getStringValue(InitOption::OPT_HA_ZONE_NAME);
  }

  msgbrokerdrv = GlobalConfiguration::getInstance()->getOption<std::string>(InitOption::OPT_MSG_BROKER_DRIVER);

  cons = chaos::common::message::MessagePSDriver::getConsumerDriver(msgbrokerdrv, groupid);
}
void QueryDataMsgPSConsumer::messageHandler(chaos::common::message::ele_t& data) {
  
  try {
    chaos::common::data::CDataWrapper* cd=data.cd.get();

    if (cd&&cd->hasKey(DataPackCommonKey::DPCK_DATASET_TYPE) && cd->hasKey(NodeDefinitionKey::NODE_UNIQUE_ID)) {
      uint64_t now = TimingUtil::getTimeStamp();

      int pktype = cd->getInt32Value(DataPackCommonKey::DPCK_DATASET_TYPE);

      int64_t  ts = 0;
      uint32_t st = (uint32_t)DataServiceNodeDefinitionType::DSStorageTypeLive;
      if (cd->hasKey(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE)) {
        st = cd->getInt32Value(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE);
        if (pktype != DataPackCommonKey::DPCK_DATASET_TYPE_OUTPUT) {
          st |= (uint32_t)DataServiceNodeDefinitionType::DSStorageTypeLive;
        }
      }

    //  kp          = cd->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID) + datasetTypeToPostfix(pktype);
      int32_t lat = 0;
      if (pktype == DataPackCommonKey::DPCK_DATASET_TYPE_LOG) {
        if (cd->hasKey(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP)) {
          ts  = cd->getInt64Value(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP);
          lat = now - ts;
          if (lat > chaos::common::constants::SkipDatasetOlderThan) {
            ERR <<  data.key << " log too old: " << lat/1000.0 << " s, skipping...";
            return;
          }
          cd->addInt32Value(DataPackCommonKey::NODE_MDS_TIMEDIFF, lat);
        }
        //  DBG<<"Queue:"<<CObjectProcessingPriorityQueue<CDataWrapper>::queueSize()<<" LOG:"<<cd->getJSONString();
        if (CObjectProcessingPriorityQueue<CDataWrapper>::queueSize() < MAX_LOG_QUEUE) {
          CDWShrdPtr ptr(cd->clone().release());
          CObjectProcessingPriorityQueue<CDataWrapper>::push(ptr, 0);
        } else {
          ERR <<  data.key << "] too many logs on queue for DB:" << CObjectProcessingPriorityQueue<CDataWrapper>::queueSize();
          return;
        }
      } else if (cd->hasKey(DataPackCommonKey::DPCK_TIMESTAMP)) {
        ts  = cd->getInt64Value(DataPackCommonKey::DPCK_TIMESTAMP);
        lat = (now - ts);
        if ((pktype == DataPackCommonKey::DPCK_DATASET_TYPE_HEALTH)) {
          if (lat > (chaos::common::constants::HBTimersTimeoutinMSec * 2)) {
            // health too old
            return;
          }
        } else if ((pktype == DataPackCommonKey::DPCK_DATASET_TYPE_OUTPUT) || (pktype == DataPackCommonKey::DPCK_DATASET_TYPE_INPUT)) {
          if (((st == 0) || (st == DataServiceNodeDefinitionType::DSStorageTypeLive))) {
            if (lat > chaos::common::constants::SkipDatasetOlderThan) {
              ERR <<  data.key << " too old: " << lat/1000.0 << " s, skipping...";
              // output too old
              return;
            }
            cd->removeKey(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE);
            cd->removeKey(DataPackCommonKey::DPCK_DATASET_TYPE);
          }
        }
        cd->addInt32Value(DataPackCommonKey::NODE_MDS_TIMEDIFF, lat);
      }
      ChaosStringSetConstSPtr meta_tag_set;

      if (cd->hasKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_TAG)) {
        ChaosStringSet* tag = new ChaosStringSet();
        tag->insert(cd->getStringValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_TAG));
        meta_tag_set.reset(tag);
      }
      QueryDataConsumer::consumePutEvent(cd->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID) + datasetTypeToPostfix(pktype), (uint8_t)st, meta_tag_set, *cd);
    }
  } catch (const chaos::CException& e) {
    ERR << "Chaos Exception caught processing key:" << data.key << " (" << data.off << "," << data.par << ") error:" << e.what();
  } catch (...) {
    ERR << "Unknown Exception caught processing key:" << data.key << " (" << data.off << "," << data.par << ")";
  }
}
void QueryDataMsgPSConsumer::processBufferElement(chaos::common::data::CDWShrdPtr log_entry) {
  chaos::metadata_service::api::logging::SubmitEntryBase se;
  //      DBG<<"Queue:"<<CObjectProcessingPriorityQueue<CDataWrapper>::queueSize()<<" WRITE ";

  se.execute(log_entry->clone());
}

void QueryDataMsgPSConsumer::messageError(chaos::common::message::ele_t& data) {
  ChaosStringSetConstSPtr   meta_tag_set;
  boost::mutex::scoped_lock ll(map_m);
  std::string               path = data.key;
  std::replace(path.begin(), path.end(), '.', '/');

  //  std::map<std::string, uint64_t>::iterator i=alive_map.find(path);
  if (data.cd.get() && data.cd->hasKey("msg") && data.cd->hasKey("err")) {
    ERR << "key:" << data.key << " [" << path << "] err msg:" << data.cd->getStringValue("msg") << " err:" << data.cd->getInt32Value("err");
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
  DBG << "Initialize, my broker is:" << msgbroker;

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
void QueryDataMsgPSConsumer::subscribeProcess(int attempt) {
  DBG << "Starting SubscribeProcess";

  api::node::NodeSearch node;
  sleep(10);

  while (attempt--) {
    std::vector<std::string> nodes = node.search("", (chaos::NodeType::NodeSearchType)(((int)chaos::NodeType::node_type_ceu) | ((int)chaos::NodeType::node_type_agent) | ((int)chaos::NodeType::node_type_us)));  // search CEU

    DBG << "] Found " << nodes.size() << " to subscribe";

    for (std::vector<std::string>::iterator i = nodes.begin(); i != nodes.end(); i++) {
      if (i->size()) {
        DBG << "] Subscribing to:" << *i;

        if (cons->subscribe(*i) != 0) {
          ERR << " cannot subscribe to :" << *i << " err:" << cons->getLastError();

        } else {
          DBG << "] Subscribed to:" << *i;
        }
      }
    }
  }
}
void QueryDataMsgPSConsumer::start() {
  DBG << "Starting Msg consumer";

  cons->start();
  boost::thread(&QueryDataMsgPSConsumer::subscribeProcess, this, 1);

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

  if (channel_data.get() == NULL || channel_data->data() == NULL) {
    // DBG<<"Empty health for:\""<<key<<"\" registration pack";
    if (alive_map.find(key) == alive_map.end()) {
      if (cons->subscribe(key) != 0) {
        ERR << "] cannot subscribe to :" << key << " err:" << cons->getLastError();

      } else {
        alive_map[key] = TimingUtil::getTimeStamp();

        DBG << "] Subscribed to:" << key << " at:" << alive_map[key];
      }
    }
    return 0;
  }
  CDataWrapper health_data_pack((char*)channel_data->data());

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

}  // namespace metadata_service
}  // namespace chaos