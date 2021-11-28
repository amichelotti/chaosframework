/*
 * Copyright 2020 INFN
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

#include "IODirectIOPSMsgDriver.h"
#include <chaos/common/configuration/GlobalConfiguration.h>
#include <chaos/common/io/QueryCursorRPC.h>
#include <chaos/common/message/MDSMessageChannel.h>
#define IODirectIOPSMsgDriver_LINFO_ INFO_LOG(IODirectIOPSMsgDriver)
#define IODirectIOPSMsgDriver_DLDBG_ DBG_LOG(IODirectIOPSMsgDriver)
#define IODirectIOPSMsgDriver_LERR_ ERR_LOG(IODirectIOPSMsgDriver)

using namespace chaos;
using namespace chaos::common::io;
using namespace chaos::common::data;
using namespace chaos::common::utility;
using namespace chaos::common::network;
using namespace chaos::DataType;
using namespace std;
using namespace boost;

namespace chaos_data        = chaos::common::data;
namespace chaos_dio         = chaos::common::direct_io;
namespace chaos_dio_channel = chaos::common::direct_io::channel;

// std::map<std::string,chaos::common::message::msgHandler> IODirectIOPSMsgDriver::handler_map;
// boost::mutex IODirectIOPSMsgDriver::hmutex;
DEFINE_CLASS_FACTORY(IODirectIOPSMsgDriver, IODataDriver);

// using namespace memcache;
IODirectIOPSMsgDriver::IODirectIOPSMsgDriver(const std::string& alias)
    : IODirectIODriver(alias), have_direct_cache(-1),mds_channel(NULL) {
  IODirectIOPSMsgDriver_DLDBG_ << "Instantiate:" << alias;
  msgbrokerdrv = "kafka-rdk";
  msgbrokerdrv = GlobalConfiguration::getInstance()->getOption<std::string>(InitOption::OPT_MSG_BROKER_DRIVER);

  prod            = chaos::common::message::MessagePSDriver::getProducerDriver(msgbrokerdrv);
  std::string gid = GlobalConfiguration::getInstance()->getNodeUID();

  if (gid == "") {
    gid = "IODirectIODriver";
  }
  cons = chaos::common::message::MessagePSDriver::getConsumerDriver(msgbrokerdrv, gid);
  if (cons->handlersEmpty()) {
    cons->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONARRIVE, boost::bind(&IODirectIOPSMsgDriver::defaultHandler, this, _1));
  }
  mds_channel=chaos::common::network::NetworkBroker::getInstance()->getMetadataserverMessageChannel();
}

IODirectIOPSMsgDriver::~IODirectIOPSMsgDriver() {
  // SO that if used as shared pointer will be called once the object is destroyed
  if(mds_channel){
    chaos::common::network::NetworkBroker::getInstance()->disposeMessageChannel(mds_channel);
  }
}
int IODirectIOPSMsgDriver::storeHealthData(const std::string&                           key,
                                           CDWShrdPtr                                   data_to_store,
                                           DataServiceNodeDefinitionType::DSStorageType storage_type,
                                           const ChaosStringSet&                        tag_set) {
  // IODirectIODriver::storeHealthData(key,data_to_store,storage_type,tag_set);
  return storeData(key, data_to_store, DataServiceNodeDefinitionType::DSStorageTypeLive, tag_set);
}
void IODirectIOPSMsgDriver::init(void* _init_parameter) {
  IODirectIODriver::init(_init_parameter);
  if (GlobalConfiguration::getInstance()->getConfiguration()->hasKey(InitOption::OPT_MSG_BROKER_SERVER)) {
    msgbroker = GlobalConfiguration::getInstance()->getConfiguration()->getStringValue(InitOption::OPT_MSG_BROKER_SERVER);
    prod->addServer(msgbroker);

    if (prod->applyConfiguration() != 0) {
      throw chaos::CException(-1, "cannot initialize Publish Subscribe Producer:" + prod->getLastError(), __PRETTY_FUNCTION__);
    }
    prod->start();
  }
}
void IODirectIOPSMsgDriver::defaultHandler(chaos::common::message::ele_t& data) {
  std::map<std::string, chaos::common::message::msgHandler>::iterator i, end;
  {
    ChaosLockGuard ll(hmutex);
    i   = handler_map.find(data.key);
    end = handler_map.end();
  }
  if (i != end) {
    IODirectIOPSMsgDriver_DLDBG_ << "calling handler associated to:" << data.key;

    i->second(data);
  }
  IODirectIOPSMsgDriver_DLDBG_ << " message from:" << data.key << " no handler found among:" << handler_map.size();

  return;
}

int IODirectIOPSMsgDriver::subscribe(const std::string& key) {
  int ret = -1;
  if (cons.get() != NULL) {
    ret = cons->subscribe(key);
    if (ret == 0) {
      cons->start();
    }
  }

  return ret;
}
int IODirectIOPSMsgDriver::addHandler(const std::string& key, chaos::common::message::msgHandler cb) {
  std::string topic = key;
  std::replace(topic.begin(), topic.end(), '/', '.');
  ChaosLockGuard ll(hmutex);
  handler_map[topic] = cb;
  IODirectIOPSMsgDriver_DLDBG_ << handler_map.size() << "] adding handler for:" << topic;

  return 0;
}
int IODirectIOPSMsgDriver::addHandler(chaos::common::message::msgHandler cb) {
  cons->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONARRIVE, cb);
  return 0;
}

void IODirectIOPSMsgDriver::deinit() {
  IODirectIODriver::deinit();
  if(prod.get()){
    prod->stop();
  }
  if (cons.get()) {
    cons->stop();
  }
  IODirectIOPSMsgDriver_DLDBG_ << "Deinitialized";
}

int IODirectIOPSMsgDriver::storeData(const std::string&                           key,
                                     CDWShrdPtr&                                  data_to_store,
                                     DataServiceNodeDefinitionType::DSStorageType storage_type,
                                     const ChaosStringSet&                        tag_set) {
  int err = 0;
  if (data_to_store.get() == NULL) {
    IODirectIOPSMsgDriver_LERR_ << "Packet not allocated";
    return -100;
  }
  if (storage_type != DataServiceNodeDefinitionType::DSStorageTypeUndefined) {
    if (!data_to_store->hasKey(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE)) {
      data_to_store->addInt32Value(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE, storage_type);
    } else {
      data_to_store->setValue(DataServiceNodeDefinitionKey::DS_STORAGE_TYPE, storage_type);
    }

    if (tag_set.size()) {
      if (!data_to_store->hasKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_TAG)) {
        data_to_store->addStringValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_TAG, *tag_set.begin());
      } else {
        data_to_store->setValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_TAG, *tag_set.begin());
      }
    }
  }
  if (data_to_store->hasKey(NodeDefinitionKey::NODE_UNIQUE_ID)) {
    if ((err = prod->pushMsgAsync(*data_to_store.get(), data_to_store->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID))) != 0) {
      DEBUG_CODE(IODirectIOPSMsgDriver_LERR_ << "Error pushing " << prod->getLastError());
    }
  } else {
    if ((err = prod->pushMsgAsync(*data_to_store.get(), key)) != 0) {
      DEBUG_CODE(IODirectIOPSMsgDriver_LERR_ << "Error pushing key:" << key << " error:" << prod->getLastError());
    }
  }

  return err;
}

chaos::common::data::CDataWrapper* IODirectIOPSMsgDriver::updateConfiguration(chaos::common::data::CDataWrapper* newConfigration) {
  // lock the feeder access
  // checkif someone has passed us the device indetification
  if (newConfigration == NULL) {
    IODirectIOPSMsgDriver_LERR_ << "Invalid configuration";

    return NULL;
  }
  if (cache_driver.get() == NULL) {
    if (newConfigration->hasKey(chaos::DataServiceNodeDefinitionKey::DS_CACHE_SETTINGS)) {
      CDWUniquePtr cs;

      cs = newConfigration->getCSDataValue(chaos::DataServiceNodeDefinitionKey::DS_CACHE_SETTINGS);
      if (cs->hasKey(chaos::common::cache_system::OPT_CACHE_DRIVER)) {
        IODirectIOPSMsgDriver_DLDBG_ << "direct cache params:" << cs->getJSONString();
        setpar.init(*cs.get());
      }
    }
  }
  if (newConfigration->hasKey(DataServiceNodeDefinitionKey::DS_BROKER_ADDRESS_LIST)) {
    chaos_data::CMultiTypeDataArrayWrapperSPtr liveMemAddrConfig               = newConfigration->getVectorValue(DataServiceNodeDefinitionKey::DS_BROKER_ADDRESS_LIST);
    size_t                                     numerbOfserverAddressConfigured = liveMemAddrConfig->size();
    for (int idx = 0; idx < numerbOfserverAddressConfigured; idx++) {
      string serverDesc = liveMemAddrConfig->getStringElementAtIndex(idx);
      IODirectIOPSMsgDriver_DLDBG_ << CHAOS_FORMAT("Adding broker %1% to IODirectIOPSMsgDriver", % serverDesc);

      if (msgbroker.size() == 0) {
        prod->addServer(serverDesc);

        if (prod->applyConfiguration() != 0) {
          throw chaos::CException(-1, "cannot initialize Publish Subscribe Producer:" + prod->getLastError(), __PRETTY_FUNCTION__);
        }
        prod->start();
        msgbroker = serverDesc;
        if (cons.get() != NULL) {
          cons->addServer(msgbroker);
          if (cons->applyConfiguration() != 0) {
            throw chaos::CException(-1, "cannot initialize Publish Subscribe Consumer:" + prod->getLastError(), __PRETTY_FUNCTION__);
          }
        }
      }
      IODirectIOPSMsgDriver_DLDBG_ << CHAOS_FORMAT("Added broker %1% to IODirectIOPSMsgDriver", % serverDesc);

      // add new url to connection feeder, thi method in case of failure to allocate service will throw an eception
    }
  }
  if (newConfigration->hasKey(DataServiceNodeDefinitionKey::DS_SUBSCRIBE_KEY_LIST)) {
    if (cons.get()) {
      chaos_data::CMultiTypeDataArrayWrapperSPtr liveMemAddrConfig               = newConfigration->getVectorValue(DataServiceNodeDefinitionKey::DS_SUBSCRIBE_KEY_LIST);
      size_t                                     numerbOfserverAddressConfigured = liveMemAddrConfig->size();
      for (int idx = 0; idx < numerbOfserverAddressConfigured; idx++) {
        string key = liveMemAddrConfig->getStringElementAtIndex(idx);
        cons->subscribe(key);
      }
    }
  }
  chaos::common::data::CDataWrapper* ret = IODirectIODriver::updateConfiguration(newConfigration);

  return ret;
}

int IODirectIOPSMsgDriver::removeData(const std::string& key,
                                      uint64_t           start_ts,
                                      uint64_t           end_ts) {
  return mds_channel->deleteDataCloud(key, start_ts, end_ts);
}
/**
 *
 */
int IODirectIOPSMsgDriver::retriveMultipleData(const ChaosStringVector&               key,
                                               chaos::common::data::VectorCDWShrdPtr& result) {
  tryCacheInit();

  if (cache_driver.get()) {
    result = cache_driver->getData(key);
    return 0;
  }
  return mds_channel->retriveMultipleData(key, result);
}

void IODirectIOPSMsgDriver::tryCacheInit() {
  if (have_direct_cache == -1) {
    have_direct_cache = 0;

    std::string cache_impl_name = setpar.cache_driver_impl;
    IODirectIOPSMsgDriver_DLDBG_ << "Try initialize Cache:" << cache_impl_name;

    cache_driver.reset(ObjectFactoryRegister<chaos::common::cache_system::CacheDriver>::getInstance()->getNewInstanceByName(cache_impl_name),
                       cache_impl_name);
    if (cache_driver.get()) {
      try {
        cache_driver.init((void*)&setpar, __PRETTY_FUNCTION__);
        IODirectIOPSMsgDriver_DLDBG_ << "direct cache initialized";
        have_direct_cache = 1;
      } catch (CException& e) {
        cache_driver.reset(NULL, cache_impl_name);
        throw e;
      } catch (...) {
        cache_driver.reset(NULL, cache_impl_name);
      }
    } else {
      IODirectIOPSMsgDriver_DLDBG_ << "cache driver " << cache_impl_name << " NOT found";
    }
    // try to initialize
  }
}

/*
 * retrieveRawData
 */
CDWUniquePtr IODirectIOPSMsgDriver::retrieveData(const std::string& key) {
  tryCacheInit();
  if (cache_driver.get()) {
    chaos::common::data::CDWShrdPtr r = cache_driver->getData(key);
    if (r.get()) {
      return r->clone();
    }
    return CDWUniquePtr();
  }
  return mds_channel->retrieveData(key);
}

int IODirectIOPSMsgDriver::loadDatasetTypeFromSnapshotTag(const std::string&      restore_point_tag_name,
                                                          const std::string&      key,
                                                          uint32_t                dataset_type,
                                                          chaos_data::CDWShrdPtr& cdw_shrd_ptr) {
  // return IODirectIODriver::loadDatasetTypeFromSnapshotTag(restore_point_tag_name,key,dataset_type,cdw_shrd_ptr);
  chaos::common::data::CDataWrapper data_set;
  int                               err = mds_channel->loadSnapshotNodeDataset(restore_point_tag_name, key, data_set);
  // IODirectIOPSMsgDriver_DLDBG_<<"SNAPSHOT:"<<data_set.getJSONString();
  if ((dataset_type == DatasetTypeInput) && data_set.hasKey(DataPackID::INPUT_DATASET_ID) && data_set.isCDataWrapperValue(DataPackID::INPUT_DATASET_ID)) {
    cdw_shrd_ptr.reset(data_set.getCSDataValue(DataPackID::INPUT_DATASET_ID).release());

  } else if ((dataset_type == DatasetTypeOutput) && data_set.hasKey(DataPackID::OUTPUT_DATASET_ID) && data_set.isCDataWrapperValue(DataPackID::OUTPUT_DATASET_ID)) {
    cdw_shrd_ptr.reset(data_set.getCSDataValue(DataPackID::OUTPUT_DATASET_ID).release());

  } else {
    IODirectIOPSMsgDriver_LERR_ << " NOR INPUT OR OUTPUT snapshot selected " << data_set.getJSONString();
    //  cdw_shrd_ptr.reset(data_set.clone().release());
  }
  if (cdw_shrd_ptr.get()) {
    IODirectIOPSMsgDriver_DLDBG_ << "SNAPSHOT type:" << dataset_type << " VAL:" << cdw_shrd_ptr->getJSONString();
  }
  return err;
}

QueryCursor* IODirectIOPSMsgDriver::performQuery(const std::string&    key,
                                                 const uint64_t        start_ts,
                                                 const uint64_t        end_ts,
                                                 const ChaosStringSet& meta_tags,
                                                 const ChaosStringSet& projection_keys,
                                                 const uint32_t        page_len) {
  // IODirectIOPSMsgDriver_DLDBG_<<"query "<<key<<" start:"<<start_ts<<" end:"<<end_ts;
  QueryCursor* q = new QueryCursorRPC(UUIDUtil::generateUUID(),
                                      key,
                                      start_ts,
                                      end_ts,
                                      meta_tags,
                                      projection_keys,
                                      page_len);
  if (q) {
    // add query to map
    ChaosWriteLock wmap_loc(map_query_future_mutex);
    map_query_future.insert(make_pair(q->queryID(), q));
  } else {
    releaseQuery(q);
  }
  return q;
}

QueryCursor* IODirectIOPSMsgDriver::performQuery(const std::string&    key,
                                                 const uint64_t        start_ts,
                                                 const uint64_t        end_ts,
                                                 const uint64_t        sequid,
                                                 const uint64_t        runid,
                                                 const ChaosStringSet& meta_tags,
                                                 const ChaosStringSet& projection_keys,
                                                 uint32_t              page_len) {
  //  IODirectIOPSMsgDriver_DLDBG_<<"query "<<key<<" start:"<<start_ts<<" end:"<<end_ts;

  QueryCursor* q = new QueryCursorRPC(UUIDUtil::generateUUID(),
                                      key,
                                      start_ts,
                                      end_ts,
                                      sequid,
                                      runid,
                                      meta_tags,
                                      projection_keys,
                                      page_len);
  if (q) {
    // add query to map
    ChaosWriteLock wmap_loc(map_query_future_mutex);
    map_query_future.insert(make_pair(q->queryID(), q));
  } else {
    releaseQuery(q);
  }
  return q;
}
