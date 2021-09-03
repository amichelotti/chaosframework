/*
 * File:   ChaosManager.cpp
 * Author: Andrea Michelotti
 *
 * Created on 21/04/2021
 */
#include "ChaosManager.h"
#include <chaos/common/message/MDSMessageChannel.h>
#include <ChaosMetadataService/api/node/ClearCommandQueue.h>
#include <ChaosMetadataService/api/node/CommandTemplateSubmit.h>
#include <ChaosMetadataService/api/node/KillCurrentCommand.h>
#include <ChaosMetadataService/api/node/NodeGetDescription.h>
#include <ChaosMetadataService/api/node/NodeNewDelete.h>
#include <ChaosMetadataService/api/node/NodeSearch.h>
#include <ChaosMetadataService/api/node/NodeSetDescription.h>
#include <ChaosMetadataService/api/node/UpdateProperty.h>

#include <ChaosMetadataService/api/unit_server/GetSetFullUnitServer.h>
#include <ChaosMetadataService/api/unit_server/LoadUnloadControlUnit.h>
#include <ChaosMetadataService/api/unit_server/ManageCUType.h>

#include <ChaosMetadataService/api/agent/CheckAgentHostedProcess.h>
#include <ChaosMetadataService/api/agent/GetAgentForNode.h>
#include <ChaosMetadataService/api/agent/ListNodeForAgent.h>
#include <ChaosMetadataService/api/agent/LoadAgentDescription.h>
#include <ChaosMetadataService/api/agent/LoadNodeAssociation.h>
#include <ChaosMetadataService/api/agent/NodeOperation.h>
#include <ChaosMetadataService/api/agent/SaveNodeAssociation.h>

#include <ChaosMetadataService/api/script/LoadFullScript.h>
#include <ChaosMetadataService/api/script/ManageScriptInstance.h>
#include <ChaosMetadataService/api/script/RemoveScript.h>
#include <ChaosMetadataService/api/script/SaveScript.h>
#include <ChaosMetadataService/api/script/SearchScript.h>
#include <ChaosMetadataService/api/service/CreateNewSnapshot.h>
#include <ChaosMetadataService/api/service/DeleteSnapshot.h>
#include <ChaosMetadataService/api/service/GetAllSnapshot.h>
#include <ChaosMetadataService/api/service/GetNodesForSnapshot.h>
#include <ChaosMetadataService/api/service/GetSnapshotDatasetsForNode.h>
#include <ChaosMetadataService/api/service/GetSnapshotForNode.h>
#include <ChaosMetadataService/api/service/RestoreSnapshot.h>
#include <ChaosMetadataService/api/service/SetSnapshotDatasetsForNode.h>

#include <ChaosMetadataService/api/control_unit/Delete.h>
#include <ChaosMetadataService/api/control_unit/DeleteInstance.h>
#include <ChaosMetadataService/api/control_unit/GetFullDescription.h>
#include <ChaosMetadataService/api/control_unit/GetInstance.h>
#include <ChaosMetadataService/api/control_unit/InitDeinit.h>
#include <ChaosMetadataService/api/control_unit/SetInputDatasetAttributeValues.h>
#include <ChaosMetadataService/api/control_unit/SetInstanceDescription.h>
#include <ChaosMetadataService/api/control_unit/SendStorageBurst.h>

#include <ChaosMetadataService/api/control_unit/StartStop.h>
#include <ChaosMetadataService/api/service/GetVariable.h>
#include <ChaosMetadataService/api/service/RemoveVariable.h>
#include <ChaosMetadataService/api/service/SetVariable.h>

#include <ChaosMetadataService/api/logging/SearchLogEntry.h>
#include <chaos_service_common/DriverPoolManager.h>

using namespace chaos::common::cache_system;
using namespace chaos::common::data;
using namespace chaos::service_common;
using namespace chaos::common::utility;
using namespace chaos::metadata_service::api::script;
using namespace chaos::metadata_service::batch;
using namespace chaos::common::batch_command;

using namespace chaos::metadata_service::api::node;
using namespace chaos::metadata_service::api::control_unit;
using namespace chaos::metadata_service::api::unit_server;
using namespace chaos::metadata_service::api::agent;
using namespace chaos::metadata_service::api::logging;
using namespace chaos::metadata_service::api::service;

#define DBGET DBG_LOG(ChaosManager)
#define DBGETERR ERR_LOG(ChaosManager)
#define CTRLDBG_ DBG_LOG(ChaosManager)
#define CTRLERR_ ERR_LOG(ChaosManager)
#define CALC_EXEC_START \
  uint64_t reqtime = chaos::common::utility::TimingUtil::getTimeStampInMicroseconds();

#define CALC_EXEC_END                                                                       \
  tot_us += (chaos::common::utility::TimingUtil::getTimeStampInMicroseconds() - reqtime);   \
  naccess++;                                                                                \
  if (naccess % 500 == 0) {                                                                 \
    double refresh = tot_us / 500.0;                                                        \
    tot_us         = 0;                                                                     \
    DBGET << " Profiling: N accesses:" << naccess << " response time:" << refresh << " us"; \
  }                                                                                         \
  DBGET << " Duration: " << (chaos::common::utility::TimingUtil::getTimeStampInMicroseconds() - reqtime) << " us";

CDWShrdPtr ChaosManager::getLiveChannel(const std::string& key) {
  ChaosSharedPtr<chaos::common::data::CDataWrapper> ret;
  if (cache_driver) {
    return cache_driver->getData(key);
  }
  return ret;
}
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
    : cache_driver(NULL), persistence_driver(NULL),storage_driver(NULL) {
  if (init(conf) != 0) {
    throw chaos::CException(-1, "Cannot initialize ", __PRETTY_FUNCTION__);
  }
}
ChaosManager::ChaosManager()
    : cache_driver(NULL), persistence_driver(NULL),storage_driver(NULL) {
  chaos::common::message::MDSMessageChannel* mdsChannel = chaos::common::network::NetworkBroker::getInstance()->getMetadataserverMessageChannel();
  if (mdsChannel) {
    CDWUniquePtr best_available_da_ptr;
    if (!mdsChannel->getDataDriverBestConfiguration(best_available_da_ptr, 5000)) {
      //InizializableService::initImplementation(chaos::service_common::ChaosManager::getInstance(), (void*)best_available_da_ptr.get(), "ChaosManager", __PRETTY_FUNCTION__);
      chaos::common::network::NetworkBroker::getInstance()->disposeMessageChannel(mdsChannel);

      if (best_available_da_ptr.get()) {
        if (init(*best_available_da_ptr) != 0) {
          throw chaos::CException(-1, "Cannot initialize ", __PRETTY_FUNCTION__);
        }
      } else {
        throw chaos::CException(-2, "Cannot retrive valid configuration ", __PRETTY_FUNCTION__);
      }
    } else {
      chaos::common::network::NetworkBroker::getInstance()->disposeMessageChannel(mdsChannel);

      throw chaos::CException(-3, "Cannot retrive configuration ", __PRETTY_FUNCTION__);
    }
  } else {
      throw chaos::CException(-4, "Cannot create a MDS channel ", __PRETTY_FUNCTION__);

  }
}
ChaosManager::~ChaosManager() {
  InizializableService::deinitImplementation(DriverPoolManager::getInstance(), "DriverPoolManager", __PRETTY_FUNCTION__);
  StartableService::stopImplementation(MDSBatchExecutor::getInstance(), "MDSBatchExecutor", __PRETTY_FUNCTION__);
  StartableService::deinitImplementation(MDSBatchExecutor::getInstance(), "MDSBatchExecutor", __PRETTY_FUNCTION__);
}

int ChaosManager::init(const chaos::common::data::CDataWrapper& best_available_da_ptr) {
  CDWUniquePtr cs;

  DBGET << "Initialize parameters:" << best_available_da_ptr.getJSONString();

  if (best_available_da_ptr.hasKey(chaos::DataServiceNodeDefinitionKey::DS_CACHE_SETTINGS)) {
    cs = best_available_da_ptr.getCSDataValue(chaos::DataServiceNodeDefinitionKey::DS_CACHE_SETTINGS);
    if (cs->hasKey(OPT_CACHE_DRIVER)) {
      std::string                                     cache_impl_name = cs->getStringValue(OPT_CACHE_DRIVER) + "CacheDriver";
      chaos::common::cache_system::CacheDriverSetting setpar;

      setpar.init(*cs.get());
      DriverPoolManager::cacheSetting = setpar;
      DBGET << "direct cache parameters:" << cs->getJSONString();
    }
  }
  if (best_available_da_ptr.hasKey(chaos::DataServiceNodeDefinitionKey::DS_ADMIN_SETTINGS)) {
    cs = best_available_da_ptr.getCSDataValue(chaos::DataServiceNodeDefinitionKey::DS_ADMIN_SETTINGS);
    if (cs->hasKey(chaos::service_common::persistence::OPT_PERSITENCE_IMPL)) {
      chaos::service_common::persistence::data_access::PersistenceDriverSetting settings;

      settings.init(*cs.get());
      DriverPoolManager::persistentSetting = settings;
      DBGET << "persistent parameters:" << cs->getJSONString();
    }
  }
  if (best_available_da_ptr.hasKey(chaos::DataServiceNodeDefinitionKey::DS_STORAGE_SETTINGS)) {
    cs = best_available_da_ptr.getCSDataValue(chaos::DataServiceNodeDefinitionKey::DS_STORAGE_SETTINGS);
    if (cs->hasKey(chaos::service_common::persistence::OPT_PERSITENCE_IMPL)) {
      chaos::service_common::persistence::data_access::PersistenceDriverSetting settings;

      settings.init(*cs.get());
      DriverPoolManager::objectSetting = settings;
      DBGET << "object parameters:" << cs->getJSONString();
    }
  }
  if (best_available_da_ptr.hasKey(chaos::DataServiceNodeDefinitionKey::DS_LOG_SETTINGS)) {
    cs = best_available_da_ptr.getCSDataValue(chaos::DataServiceNodeDefinitionKey::DS_LOG_SETTINGS);
    if (cs->hasKey(chaos::service_common::persistence::OPT_PERSITENCE_IMPL)) {
      chaos::service_common::persistence::data_access::PersistenceDriverSetting settings;

      settings.init(*cs.get());
      DriverPoolManager::logSetting = settings;
      DBGET << "log parameters:" << cs->getJSONString();
    }
  }

  if (cache_driver == NULL || persistence_driver == NULL) {
    try{
    InizializableService::initImplementation(DriverPoolManager::getInstance(), NULL, "DriverPoolManager", __PRETTY_FUNCTION__);
    
    } catch(...){
            DBGETERR << "Error Initializing";

    }

    cache_driver = DriverPoolManager::getInstance()->getCacheDrvPtr();
    if (cache_driver == NULL) {
      DBGETERR << "Cannot use direct cache";
      return -1;
    } else {
      DBGET << "Using direct cache";
    }
    persistence_driver = DriverPoolManager::getInstance()->getPersistenceDrvPtr();
    if (persistence_driver == NULL) {
      DBGETERR << "Cannot use direct persistence";
      return -2;

    } else {
      DBGET << "Using direct persistence";
    }
    StartableService::initImplementation(MDSBatchExecutor::getInstance(), NULL, "MDSBatchExecutor", __PRETTY_FUNCTION__);
    StartableService::startImplementation(MDSBatchExecutor::getInstance(), "MDSBatchExecutor", __PRETTY_FUNCTION__);
    storage_driver = DriverPoolManager::getInstance()->getObjectStorageDrvPtr();
    if (storage_driver == NULL) {
      DBGETERR << "Cannot use direct storage driver";
      return -2;

    } else {
      DBGET << "Using direct storage driver";
    }

    log_driver = DriverPoolManager::getInstance()->getLogDrvPtr();
    if (log_driver == NULL) {
      DBGETERR << "Cannot use log direct driver";
      return -2;

    } else {
      DBGET << "Using log direct driver";
    }
    

  }

  return 0;
}
std::map<uint64_t, std::string> ChaosManager::getAllSnapshot(const std::string& query_filter) {
  std::map<uint64_t, std::string> snapshot_found;
  CDWUniquePtr                    res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue("snapshot_name", query_filter);

    GetAllSnapshot node;
    res = node.execute(MOVE(message));

    if (res.get() &&
        res->hasKey("snapshot_list_result") &&
        res->isVectorValue("snapshot_list_result")) {
      CMultiTypeDataArrayWrapperSPtr snapshot_desc_list = res->getVectorValue("snapshot_list_result");
      for (int idx = 0;
           idx < snapshot_desc_list->size();
           idx++) {
        ChaosUniquePtr<chaos::common::data::CDataWrapper> tmp_desc(snapshot_desc_list->getCDataWrapperElementAtIndex(idx));

        if (tmp_desc->hasKey("snap_name")) {
          if (!query_filter.empty()) {
            std::string cmp = tmp_desc->getStringValue("snap_name");
            // TODO: implement filter in DB query
            if (strstr(cmp.c_str(), query_filter.c_str())) {
              uint64_t tm        = tmp_desc->getInt64Value("snap_ts");
              snapshot_found[tm] = cmp;
            }
          } else {
            uint64_t tm        = tmp_desc->getInt64Value("snap_ts");
            snapshot_found[tm] = tmp_desc->getStringValue("snap_name");
          }
        }
      }
    }

    CALC_EXEC_END
  }
  return snapshot_found;
}

chaos::common::data::VectorCDWShrdPtr ChaosManager::getLiveChannel(const std::vector<std::string>& channels) {
  chaos::common::data::VectorCDWShrdPtr results;
  if (cache_driver) {
    results = cache_driver->getData(channels);
    return results;
  }
  return results;
}
/**** NODES */  ///
chaos::common::data::CDWUniquePtr ChaosManager::updateProperty(const std::string& uid, const chaos::common::data::CDataWrapper& value) {
  CDWUniquePtr res;
  if (!value.hasKey("properties") || !!value.isVectorValue("properties")) {
    DBGETERR << "Missing properties vector key";

    return res;
  }

  if (persistence_driver) {
    UpdateProperty node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    ChaosSharedPtr<CMultiTypeDataArrayWrapper> dw = value.getVectorValue("properties");
    for (int idx = 0; idx < dw->size(); idx++) {
      if (dw->isCDataWrapperElementAtIndex(idx)) {
      }
    }
    message->addCSDataValue("property", value);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
ChaosStringVector ChaosManager::getSnapshotForNode(const std::string& uid) {
  ChaosStringVector snapshot_found;
  if (persistence_driver) {
    GetSnapshotForNode node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);

    CDWUniquePtr res = node.execute(MOVE(message));
    if (res.get() &&
        res->hasKey("snapshot_for_node") &&
        res->isVectorValue("snapshot_for_node")) {
      //we have result
      CMultiTypeDataArrayWrapperSPtr snapshot_desc_list = res->getVectorValue("snapshot_for_node");
      for (int idx = 0;
           idx < snapshot_desc_list->size();
           idx++) {
        const std::string node_uid = snapshot_desc_list->getStringElementAtIndex(idx);
        snapshot_found.push_back(node_uid);
      }
    }
    CALC_EXEC_END
  }
  return snapshot_found;
}

ChaosStringVector ChaosManager::getNodesForSnapshot(const std::string& uid) {
  ChaosStringVector snapshot_found;
  if (persistence_driver) {
    GetNodesForSnapshot node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue("snapshot_name", uid);

    CDWUniquePtr res = node.execute(MOVE(message));
    if (res.get() &&
        res->hasKey("node_in_snapshot") &&
        res->isVectorValue("node_in_snapshot")) {
      //we have result
      CMultiTypeDataArrayWrapperSPtr snapshot_desc_list = res->getVectorValue("node_in_snapshot");
      for (int idx = 0;
           idx < snapshot_desc_list->size();
           idx++) {
        const std::string node_uid = snapshot_desc_list->getStringElementAtIndex(idx);
        snapshot_found.push_back(node_uid);
      }
    }
    CALC_EXEC_END
  }
  return snapshot_found;
}
std::map<uint64_t, std::string> ChaosManager::getAllSnapshotOfCU(const std::string& cuname) {
  std::vector<std::string>        node_found = getSnapshotForNode(cuname);
  std::map<uint64_t, std::string> res;
  for (std::vector<std::string>::iterator i = node_found.begin(); i != node_found.end(); i++) {
    std::map<uint64_t, std::string> tmp = getAllSnapshot(*i);
    res.insert(tmp.begin(), tmp.end());
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::deleteSnapshot(const std::string& snapshot_name) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue("snapshot_name", snapshot_name);
    DeleteSnapshot node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::agentNodeOperation(const std::string& node_uid, int32_t op) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, node_uid);
    message->addInt32Value("node_operation", op);
    NodeOperation node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::clearCommandQueue(const std::string& name) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, name);

    ClearCommandQueue node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::killCurrentCommand(const std::string& name) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, name);

    KillCurrentCommand node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::checkAgentHostedProcess(const std::string& name) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, name);

    CheckAgentHostedProcess node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::loadAgentDescription(const std::string& agent_uid, bool loaddata) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, agent_uid);
    message->addBoolValue("load_related_data", loaddata);

    LoadAgentDescription node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::listNodeForAgent(const std::string& agent_uid) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, agent_uid);

    ListNodeForAgent node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::loadNodeAssociation(const std::string& agent_uid, const std::string& node_association) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, agent_uid);
    message->addStringValue(AgentNodeDefinitionKey::NODE_ASSOCIATED, node_association);

    LoadNodeAssociation node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::saveNodeAssociation(const std::string& agent_uid, const chaos::common::data::CDataWrapper& node_association) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, agent_uid);
    message->addCSDataValue(AgentNodeDefinitionKey::NODE_ASSOCIATED, node_association);

    SaveNodeAssociation node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::getSnapshotDatasetForNode(const std::string& snapname, const std::string& node_uid) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue("snapshot_name", snapname);
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, node_uid);

    GetSnapshotDatasetsForNode node;
    res = node.execute(MOVE(message));
    if (res.get()) {
      std::map<uint64_t, std::string> mapsnap_res;

      CMultiTypeDataArrayWrapperSPtr snapshot_list = res->getVectorValue("dataset_list");
      if (snapshot_list->size()) {
        res->addStringValue("name", node_uid);
        res->addInt64Value("timestamp", mapsnap_res.begin()->first);
      }
      for (int idx = 0;
           idx < snapshot_list->size();
           idx++) {
        ChaosUniquePtr<chaos::common::data::CDataWrapper> snapshot_dataset_element(snapshot_list->getCDataWrapperElementAtIndex(idx));

        const std::string                                 dataset_name = snapshot_dataset_element->getStringValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_NAME);
        ChaosUniquePtr<chaos::common::data::CDataWrapper> val(snapshot_dataset_element->getCSDataValue("dataset_value"));
        if (val->hasKey(chaos::DataPackCommonKey::DPCK_DATASET_TYPE)) {
          std::string ret = datasetTypeToHuman(val->getUInt32Value(chaos::DataPackCommonKey::DPCK_DATASET_TYPE));
          res->addCSDataValue(ret, *val);
        }
      }
    }
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::setSnapshotDatasetsForNode(const std::string& snapshot_name,const std::string& uid,chaos::common::data::VectorCDWShrdPtr& datasets_value_vec){
  CDWUniquePtr res;

    CDWUniquePtr message(new CDataWrapper());
    message->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    message->addStringValue("snapshot_name", snapshot_name);

    for(chaos::common::data::VectorCDWShrdPtr::const_iterator i=datasets_value_vec.begin();i!=datasets_value_vec.end();i++){
    	message->appendCDataWrapperToArray(*(i->get()));
    }
    message->finalizeArrayForKey("dataset");
  if (persistence_driver) {
    CALC_EXEC_START;
    SetSnapshotDatasetsForNode node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::createNewSnapshot(const std::string& snapshot_name, const ChaosStringVector& node_list) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue("snapshot_name", snapshot_name);

    for (ChaosStringVectorConstIterator it  = node_list.begin(),
                                        end = node_list.end();
         it != end;
         it++) {
      message->appendStringToArray(*it);
    }
    message->finalizeArrayForKey("node_list");
    CreateNewSnapshot node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::restoreSnapshot(const std::string& snapshot_name) {
  CDWUniquePtr res;

  if (persistence_driver) {
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue("snapshot_name", snapshot_name);

    RestoreSnapshot node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::commandTemplateSubmit(const std::string& uid, const std::string& command_alias, const chaos::common::data::CDWUniquePtr& slow_command_data, const SubmissionRuleType::SubmissionRule submission_rule, const uint32_t priority, const uint64_t scheduler_steps_delay, const uint32_t submission_checker_steps_delay) {
  CDWUniquePtr res;

  CDWUniquePtr message(new CDataWrapper());
  //this key need only to inform mds to redirect to node the slowcomand without porcess it
  message->addNullValue("direct_mode");
  // set the default slow command information
  message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
  message->addStringValue(BatchCommandAndParameterDescriptionkey::BC_ALIAS, command_alias);
  message->addInt32Value(BatchCommandSubmissionKey::SUBMISSION_RULE_UI32, (uint32_t)submission_rule);
  message->addInt32Value(BatchCommandSubmissionKey::SUBMISSION_PRIORITY_UI32, (uint32_t)priority);

  if (scheduler_steps_delay) message->addInt64Value(BatchCommandSubmissionKey::SCHEDULER_STEP_TIME_INTERVALL_UI64, scheduler_steps_delay);
  if (submission_checker_steps_delay) message->addInt32Value(BatchCommandSubmissionKey::SUBMISSION_RETRY_DELAY_UI32, submission_checker_steps_delay);
  if (slow_command_data.get()) {
    message->appendAllElement(*slow_command_data);
  }
  if (persistence_driver) {
    CALC_EXEC_START;

    CommandTemplateSubmit node;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::manageCUType(const std::string& uid, const std::string& control_unit_type, int op) {
  CDWUniquePtr res;
  if (persistence_driver) {
    ManageCUType node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    message->addStringValue(UnitServerNodeDefinitionKey::UNIT_SERVER_HOSTED_CONTROL_UNIT_CLASS, control_unit_type);
    message->addInt32Value("operation", op);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::loadUnloadControlUnit(const std::string& uid, bool ini) {
  CDWUniquePtr res;
  if (persistence_driver) {
    LoadUnloadControlUnit node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    message->addBoolValue("load", ini);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::searchLogEntry(const std::string& search_string, const std::vector<std::string>& domain_list, uint64_t start_ts, uint64_t end_ts, uint64_t last_sequence_id, uint32_t page_length) {
  CDWUniquePtr res;
  if (persistence_driver) {
    SearchLogEntry node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> pack(new CDataWrapper());
    pack->addStringValue("search_string", search_string);
    if (last_sequence_id) {
      pack->addInt64Value("seq", last_sequence_id);
    }
    if (start_ts) {
      pack->addInt64Value("start_ts", start_ts);
    }
    if (end_ts) {
      pack->addInt64Value("end_ts", end_ts);
    }
    if (domain_list.size()) {
      for (ChaosStringVector::const_iterator it = domain_list.begin();
           it != domain_list.end();
           it++) {
        pack->appendStringToArray(*it);
      }
      pack->finalizeArrayForKey(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_DOMAIN);
    }
    pack->addInt32Value("page_length", page_length);
    res = node.execute(MOVE(pack));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::initDeinit(const std::string& uid, bool ini) {
  CDWUniquePtr res;
  if (persistence_driver) {
    InitDeinit node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    message->addBoolValue("init", ini);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::saveScript(const chaos::common::data::CDataWrapper& value) {
  CDWUniquePtr res;
  if (persistence_driver) {
    SaveScript node;
    CALC_EXEC_START;
    res = node.execute(MOVE(value.clone()));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::removeScript(const chaos::common::data::CDataWrapper& value) {
  CDWUniquePtr res;
  if (persistence_driver) {
    RemoveScript node;
    CALC_EXEC_START;
    res = node.execute(MOVE(value.clone()));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::setInputDatasetAttributeValues(const std::string& uid, std::map<const std::string, const std::string>& keyvalue) {
  CDWUniquePtr res;
  if (persistence_driver) {
    CDWUniquePtr message(new chaos::common::data::CDataWrapper());

    SetInputDatasetAttributeValues node;
    CDWUniquePtr                   cu_changes(new CDataWrapper());
    cu_changes->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    for (std::map<const std::string, const std::string>::iterator i = keyvalue.begin(); i != keyvalue.end(); i++) {
      CDataWrapper change;

      change.addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_NAME, i->first);
      change.addStringValue("change_value", i->second);
      cu_changes->appendCDataWrapperToArray(change);
    }
    cu_changes->finalizeArrayForKey("change_set");
    message->appendCDataWrapperToArray(*cu_changes);

    message->finalizeArrayForKey("attribute_set_values");

    CALC_EXEC_START;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::setInputDatasetAttributeValues(const std::string& uid, const std::string& key, const std::string& value) {
  CDWUniquePtr res;
  if (persistence_driver) {
    CDWUniquePtr message(new chaos::common::data::CDataWrapper());

    SetInputDatasetAttributeValues node;
    CDWUniquePtr                   cu_changes(new CDataWrapper());
    cu_changes->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    CDataWrapper change;
    change.addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_NAME, key);
    change.addStringValue("change_value", value);
    cu_changes->appendCDataWrapperToArray(change);
    cu_changes->finalizeArrayForKey("change_set");
    message->appendCDataWrapperToArray(*cu_changes);

    message->finalizeArrayForKey("attribute_set_values");

    CALC_EXEC_START;
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::loadFullDescription(const std::string& scriptID) {
  chaos::common::data::CDWUniquePtr ret;
  chaos::common::data::CDWUniquePtr data(new CDataWrapper());
  chaos::common::data::CDWUniquePtr list = searchScript(scriptID, 0, 10000);

  if (list.get() &&
      list->hasKey(chaos::MetadataServerApiKey::script::search_script::FOUND_SCRIPT_LIST) &&
      list->isVectorValue(chaos::MetadataServerApiKey::script::search_script::FOUND_SCRIPT_LIST)) {
    CMultiTypeDataArrayWrapperSPtr    scripts = list->getVectorValue(chaos::MetadataServerApiKey::script::search_script::FOUND_SCRIPT_LIST);
    int64_t                           lastid  = -1;
    chaos::common::data::CDWUniquePtr last;
    for (int cnt = 0; cnt < scripts->size(); cnt++) {
      chaos::common::data::CDWUniquePtr curr = scripts->getCDataWrapperElementAtIndex(cnt);
      if (curr->getInt64Value("seq") >= lastid) {
        lastid = curr->getInt64Value("seq");
        last.reset(curr.release());
      }
    }
    if (lastid > -1) {
      //
      chaos::common::data::CDWUniquePtr data(new CDataWrapper());
      data->addStringValue(ExecutionUnitNodeDefinitionKey::CHAOS_SBD_NAME, scriptID);
      data->addInt64Value("seq", lastid);
      ret = loadFullScript(*data);
    }
  }
  return ret;
}

chaos::common::data::CDWUniquePtr ChaosManager::loadFullScript(const chaos::common::data::CDataWrapper& value) {
  CDWUniquePtr res;
  if (persistence_driver) {
    LoadFullScript node;
    CALC_EXEC_START;
    res = node.execute(MOVE(value.clone()));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::manageScriptInstance(const chaos::common::data::CDataWrapper& value) {
  CDWUniquePtr res;
  if (persistence_driver) {
    ManageScriptInstance node;
    CALC_EXEC_START;
    res = node.execute(MOVE(value.clone()));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::searchScript(const std::string& search_string, uint64_t start_sequence_id, uint32_t page_lenght) {
  CDWUniquePtr res;
  if (persistence_driver) {
    SearchScript node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    CDWUniquePtr                                      api_data(new CDataWrapper());
    api_data->addStringValue("search_string", search_string);
    api_data->addInt64Value("last_sequence_id", start_sequence_id);
    api_data->addInt32Value("page_lenght", page_lenght);
    res = node.execute(MOVE(api_data));

    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::startStop(const std::string& uid, bool start) {
  CDWUniquePtr res;
  if (persistence_driver) {
    StartStop node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    message->addBoolValue("start", start);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::getCUInstance(const std::string& uid) {
  CDWUniquePtr res;
  if (persistence_driver) {
    GetInstance node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::deleteInstance(const std::string& uid, const std::string& parent) {
  CDWUniquePtr res;
  if (persistence_driver) {
    DeleteInstance node;
    Delete         node2;

    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    if (parent.size()) {
      message->addStringValue(NodeDefinitionKey::NODE_PARENT, parent);
      res = node.execute(MOVE(message));
    }
    res = node2.execute(MOVE(message));

    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::sendStorageBurst(const chaos::common::data::CDataWrapper& value){
  CDWUniquePtr res;
  if (persistence_driver) {
    SendStorageBurst node;
    CALC_EXEC_START;
    res = node.execute(MOVE(value.clone()));
    CALC_EXEC_END
  }
  return res;

}

chaos::common::data::CDWUniquePtr ChaosManager::setNodeDescription(const chaos::common::data::CDataWrapper& value) {
  CDWUniquePtr res;
  if (persistence_driver) {
    NodeSetDescription node;
    CALC_EXEC_START;
    res = node.execute(MOVE(value.clone()));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::setInstanceDescription(const std::string& uid, const chaos::common::data::CDataWrapper& instance_description) {
  CDWUniquePtr res;
  if (persistence_driver) {
    SetInstanceDescription node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    if (instance_description.hasKey(chaos::NodeDefinitionKey::NODE_TYPE)) {
      message->addStringValue(chaos::NodeDefinitionKey::NODE_TYPE, instance_description.getStringValue(chaos::NodeDefinitionKey::NODE_TYPE));

    } else {
      message->addStringValue(chaos::NodeDefinitionKey::NODE_TYPE, chaos::NodeType::NODE_TYPE_CONTROL_UNIT);
    }

    message->addCSDataValue("instance_description", instance_description);
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
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
chaos::common::data::CDWUniquePtr ChaosManager::nodeDelete(const std::string& uid, const std::string parent) {
  CDWUniquePtr res;
  if (persistence_driver) {
    NodeNewDelete node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addBoolValue("reset", true);
    if (parent.size()) {
      message->addStringValue(chaos::NodeDefinitionKey::NODE_PARENT, parent);
    }
    message->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

chaos::common::data::CDWUniquePtr ChaosManager::nodeNew(const std::string& uid, const chaos::common::data::CDataWrapper& value, const std::string parent) {
  CDWUniquePtr res;
  if (persistence_driver) {
    NodeNewDelete node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addBoolValue("reset", false);
    if (parent.size()) {
      message->addStringValue(chaos::NodeDefinitionKey::NODE_PARENT, parent);
    }
    message->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, uid);
    value.copyAllTo(*message);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
/****  */  ///
/**** VARIABLES */
chaos::common::data::CDWUniquePtr ChaosManager::getVariable(const std::string& variable_name) {
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
chaos::common::data::CDWUniquePtr ChaosManager::setVariable(const std::string& variable_name, const chaos::common::data::CDataWrapper& value) {
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
chaos::common::data::CDWUniquePtr ChaosManager::removeVariable(const std::string& variable_name) {
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
/********/

/****** US ****/
chaos::common::data::CDWUniquePtr ChaosManager::getFullUnitServer(const std::string& uid) {
  CDWUniquePtr res;
  if (persistence_driver) {
    GetSetFullUnitServer node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());

    message->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, uid);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}
chaos::common::data::CDWUniquePtr ChaosManager::setFullUnitServer(const std::string& uid, const chaos::common::data::CDataWrapper& value) {
  CDWUniquePtr res;
  if (persistence_driver) {
    GetSetFullUnitServer node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
    message->addBoolValue("reset", false);
    message->addCSDataValue("us_desc", value);

    message->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, uid);

    res = node.execute(MOVE(message));
    CALC_EXEC_END
  }
  return res;
}

/**************/

chaos::common::data::CDWUniquePtr ChaosManager::getAgentForNode(const std::string& uid) {
  CDWUniquePtr res;
  if (persistence_driver) {
    GetAgentForNode node;
    CALC_EXEC_START;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());

    message->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, uid);

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
                             uint32_t&                       lastid,
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
