/*
 * File:   ChaosManager.cpp
 * Author: Andrea Michelotti
 *
 * Created on 21/04/2021
 */
#include "ChaosManager.h"
#include <ChaosMetadataService/api/node/NodeGetDescription.h>
#include <ChaosMetadataService/api/node/NodeSearch.h>
#include <ChaosMetadataService/api/service/GetVariable.h>
#include <ChaosMetadataService/api/service/SetVariable.h>
#include <ChaosMetadataService/api/service/RemoveVariable.h>

#include <ChaosMetadataService/api/control_unit/GetFullDescription.h>

#include <chaos_service_common/DriverPoolManager.h>

using namespace chaos::common::cache_system;
using namespace chaos::common::data;
using namespace chaos::service_common;
using namespace chaos::common::utility;
using namespace chaos::metadata_service::api::node;
using namespace chaos::metadata_service::api::control_unit;
using namespace chaos::metadata_service::api::service;

#define DBGET DBG_LOG(ChaosManager)
#define DBGETERR ERR_LOG(ChaosManager)
#define CTRLDBG_ DBG_LOG(ChaosManager)
#define CTRLERR_ ERR_LOG(ChaosManager)
#define CALC_EXEC_START \
  uint64_t reqtime = chaos::common::utility::TimingUtil::getTimeStampInMicroseconds();

#define CALC_EXEC_END                                                                       \
  tot_us += (chaos::common::utility::TimingUtil::getTimeStampInMicroseconds()- reqtime);   \
  naccess++;                                                                                \
  if (naccess % 500 == 0) {                                                                 \
    double refresh = tot_us / 500.0;                                                        \
    tot_us         = 0;                                                                     \
    DBGET << " Profiling: N accesses:" << naccess << " response time:" << refresh << " us"; \
  }                                                                                         \
  DBGET << " Duration: " << (chaos::common::utility::TimingUtil::getTimeStampInMicroseconds()-reqtime ) << " us";

CDWShrdPtr ChaosManager::getLiveChannel(const std::string& key, int domain) {
  size_t                                            value_len = 0;
  ChaosSharedPtr<chaos::common::data::CDataWrapper> ret;
  std::string                                       lkey = key + chaos::datasetTypeToPostfix(domain);
  char*                                             value;
  if (cache_driver) {
    return cache_driver->getData(key);
  }
  return ret;
}
ChaosManager::ChaosManager(const chaos::common::data::CDataWrapper& conf)
    : cache_driver(NULL), persistence_driver(NULL) {
  init(conf);
}
ChaosManager::~ChaosManager() {
}
void ChaosManager::init(void* initd) {
  if (initd) {
    const chaos::common::data::CDataWrapper* p = (chaos::common::data::CDataWrapper*)initd;
    init(*p);
  }
}
int ChaosManager::init(const chaos::common::data::CDataWrapper& best_available_da_ptr) {
  CDWUniquePtr cs;
  DBGET << "Initialize parameters:" << best_available_da_ptr.getJSONString();

  if (best_available_da_ptr.hasKey("cache")) {
    cs = best_available_da_ptr.getCSDataValue("cache");
    if (cs->hasKey(OPT_CACHE_DRIVER)) {
      std::string                                     cache_impl_name = cs->getStringValue(OPT_CACHE_DRIVER) + "CacheDriver";
      chaos::common::cache_system::CacheDriverSetting setpar;

      setpar.init(*cs.get());
      DriverPoolManager::cacheSetting = setpar;
      DBGET << "direct cache parameters:" << cs->getJSONString();
    }
  }
  if (best_available_da_ptr.hasKey("persistence")) {
    cs = best_available_da_ptr.getCSDataValue("persistence");
    if (cs->hasKey(chaos::service_common::persistence::OPT_PERSITENCE_IMPL)) {
      chaos::service_common::persistence::data_access::PersistenceDriverSetting settings;

      settings.init(*cs.get());
      DriverPoolManager::persistentSetting = settings;
      DBGET << "persistent parameters:" << cs->getJSONString();
    }
  }

  if (cache_driver == NULL || persistence_driver == NULL) {
    InizializableService::initImplementation(DriverPoolManager::getInstance(), NULL, "DriverPoolManager", __PRETTY_FUNCTION__);

    cache_driver = DriverPoolManager::getInstance()->getCacheDrvPtr();
    if (cache_driver == NULL) {
      DBGETERR << "Cannot use direct cache";

    } else {
      DBGET << "Using direct cache";
    }
    persistence_driver = DriverPoolManager::getInstance()->getPersistenceDrvPtr();
    if (persistence_driver == NULL) {
      DBGETERR << "Cannot use direct persistence";

    } else {
      DBGET << "Using direct persistence";
    }
  }

  return 0;
}

chaos::common::data::VectorCDWShrdPtr ChaosManager::getLiveChannel(const std::vector<std::string>& channels) {
  chaos::common::data::VectorCDWShrdPtr results;
  if (cache_driver) {
    results = cache_driver->getData(channels);
    return results;
  }
  return results;
}
CDWUniquePtr ChaosManager::nodeGetDescription(const std::string& uid) {
  CDWUniquePtr res;
  if (persistence_driver) {
    NodeGetDescription node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::getVariable(const std::string& variable_name){
    CDWUniquePtr res;
  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(VariableDefinitionKey::VARIABLE_NAME_KEY,
                            variable_name);
    GetVariable node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;

}
chaos::common::data::CDWUniquePtr ChaosManager::setVariable(const std::string& variable_name,const chaos::common::data::CDataWrapper& value ){
     CDWUniquePtr res;
  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(VariableDefinitionKey::VARIABLE_NAME_KEY,
                            variable_name);
    message->addCSDataValue(VariableDefinitionKey::VARIABLE_VALUE_KEY,
                            value);
                        
    SetVariable node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::removeVariable(const std::string& variable_name){
   CDWUniquePtr res;
  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(VariableDefinitionKey::VARIABLE_NAME_KEY,
                            variable_name);
    RemoveVariable node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;

}

    
CDWUniquePtr ChaosManager::cuGetFullDescription(const std::string& uid) {
  CDWUniquePtr res;
  if (persistence_driver) {
    GetFullDescription node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
int ChaosManager::nodeSearch(const std::string&              unique_id_filter,
                             chaos::NodeType::NodeSearchType node_type_filter,
                             bool                            alive_only,
                             unsigned int                    last_node_sequence_id,
                             unsigned int                    page_length,
                             uint64_t&                       lastid,
                             ChaosStringVector&              node_found,
                             uint32_t                        millisec_to_wait,
                             const std::string&              impl) {
  if (persistence_driver) {
    NodeSearch nodesearch;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue("unique_id_filter", unique_id_filter);
    if (impl.size() > 0)
      message->addStringValue("impl", impl);

    message->addInt32Value("node_type_filter", (int32_t)node_type_filter);
    message->addInt32Value("last_node_sequence_id", last_node_sequence_id);
    message->addBoolValue("alive_only", alive_only);
    message->addInt32Value("result_page_length", page_length);
    CDWUniquePtr res = nodesearch.execute(MOVE(message));

    if (res.get() &&
        res->hasKey(chaos::NodeType::NODE_SEARCH_LIST_KEY) &&
        res->isVectorValue(chaos::NodeType::NODE_SEARCH_LIST_KEY)) {
      //we have result
      CMultiTypeDataArrayWrapperSPtr snapshot_desc_list = res->getVectorValue(chaos::NodeType::NODE_SEARCH_LIST_KEY);
      for (int idx = 0;
           idx < snapshot_desc_list->size();
           idx++) {
        ChaosUniquePtr<chaos::common::data::CDataWrapper> element(snapshot_desc_list->getCDataWrapperElementAtIndex(idx));
        if (element.get() &&
            element->hasKey(NodeDefinitionKey::NODE_UNIQUE_ID)) {
          node_found.push_back(element->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID));
          if (element->hasKey("seq")) {
            lastid = element->getInt64Value("seq");
          }
        }
      }
    }
    CALC_EXEC_END;
    return 0;
  }
  return -1;
}
