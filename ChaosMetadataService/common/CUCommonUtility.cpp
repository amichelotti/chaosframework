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

#include "CUCommonUtility.h"
#include <chaos_service_common/DriverPoolManager.h>
#include "../ChaosMetadataService.h"

#include <chaos/common/global.h>

#include <boost/foreach.hpp>
#include "../object_storage/object_storage.h"

using namespace chaos::common::data;
using namespace chaos::service_common;

using namespace chaos::common::property;
using namespace chaos::common::cache_system;
using namespace chaos::metadata_service::common;
using namespace chaos::metadata_service::object_storage::abstraction;
using namespace chaos::metadata_service::persistence::data_access;
using namespace chaos::service_common::persistence::data_access;

#define CUCU_INFO LOG_LOG(CUCommonUtility)
#define CUCU_DBG DBG_LOG(CUCommonUtility)
#define CUCU_ERR ERR_LOG(CUCommonUtility)

ChaosUniquePtr<chaos::common::data::CDataWrapper> CUCommonUtility::prepareRequestPackForLoadControlUnit(const std::string &cu_uid, chaos::metadata_service::persistence::data_access::NodeDataAccess *n_da, chaos::metadata_service::persistence::data_access::ControlUnitDataAccess *cu_da, chaos::metadata_service::persistence::data_access::DataServiceDataAccess *ds_da) {
  CUCU_DBG << "Prepare autoload request for:" << cu_uid;
  int                                               err     = 0;
  CDataWrapper                                     *tmp_ptr = NULL;
  ChaosUniquePtr<chaos::common::data::CDataWrapper> instance_description;
  ChaosUniquePtr<chaos::common::data::CDataWrapper> result_pack;
  if ((err = cu_da->getInstanceDescription(cu_uid,
                                           &tmp_ptr))) {
    // we haven't found an instance for the node
    CUCU_ERR << "Erroe getting instance" << err;
  } else if (tmp_ptr != NULL) {
    instance_description.reset(tmp_ptr);

    // we have instances the rpc port is got from the unit server input data of the command
    if (!instance_description->hasKey("control_unit_implementation")) {
      CUCU_ERR << "No implementaiton found";
    } else {
      CUCU_DBG << "Create the autoload datapack for:" << cu_uid;
      result_pack.reset(new CDataWrapper());

      // add cu id
      result_pack->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, cu_uid);
      // add cu type
      result_pack->addStringValue(UnitServerNodeDomainAndActionRPC::PARAM_CONTROL_UNIT_TYPE, instance_description->getStringValue("control_unit_implementation"));
      // add driver description
      instance_description->copyKeyTo(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DRIVER_DESCRIPTION, *result_pack);
      instance_description->copyKeyTo(ControlUnitNodeDefinitionKey::CONTROL_UNIT_LOAD_PARAM, *result_pack);
      addDataServicePack(result_pack, ds_da);
    }
  } else {
    CUCU_ERR << "No instance found";
  }
  return result_pack;
}

void CUCommonUtility::prepareAutoInitAndStartInAutoLoadControlUnit(const std::string     &cu_uid,
                                                                   NodeDataAccess        *n_da,
                                                                   ControlUnitDataAccess *cu_da,
                                                                   DataServiceDataAccess *ds_da,
                                                                   CDataWrapper          *auto_load_pack) {
  CUCU_DBG << "Prepare autoload request for:" << cu_uid;
  int           err     = 0;
  CDataWrapper *tmp_ptr = NULL;
  // if the control unit to load need also to be initialized and started we compose the startup command to achieve this
  if ((err = cu_da->getInstanceDescription(cu_uid, &tmp_ptr))) {
    LOG_AND_TROW_FORMATTED(CUCU_ERR, err, "Error %1% durring fetch of instance for unit server %2%", % err % cu_da)
  } else if (tmp_ptr) {
    ChaosUniquePtr<chaos::common::data::CDataWrapper> auto_inst(tmp_ptr);
    bool                                              auto_init  = auto_inst->hasKey("auto_init") ? auto_inst->getBoolValue("auto_init") : false;
    bool                                              auto_start = auto_inst->hasKey("auto_start") ? auto_inst->getBoolValue("auto_start") : false;

    if (auto_init || auto_start) {
      if (auto_init) {
        ChaosUniquePtr<chaos::common::data::CDataWrapper> init_datapack = initDataPack(cu_uid,
                                                                                       n_da,
                                                                                       cu_da,
                                                                                       ds_da);

        ChaosUniquePtr<chaos::common::data::CDataWrapper> init_message_datapack(new CDataWrapper());
        init_message_datapack->addStringValue(RpcActionDefinitionKey::CS_CMDM_ACTION_NAME, NodeDomainAndActionRPC::ACTION_NODE_INIT);
        if (init_datapack.get()) {
          init_message_datapack->addCSDataValue(RpcActionDefinitionKey::CS_CMDM_ACTION_MESSAGE, *init_datapack);
        }
        if (init_message_datapack.get()) {
          auto_load_pack->appendCDataWrapperToArray(*init_message_datapack);
        }
      }
      if (auto_start) {
        ChaosUniquePtr<chaos::common::data::CDataWrapper> start_datapack(new CDataWrapper());
        start_datapack->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, cu_uid);

        ChaosUniquePtr<chaos::common::data::CDataWrapper> start_message_datapack(new CDataWrapper());
        start_message_datapack->addStringValue(RpcActionDefinitionKey::CS_CMDM_ACTION_NAME, NodeDomainAndActionRPC::ACTION_NODE_START);
        start_message_datapack->addCSDataValue(RpcActionDefinitionKey::CS_CMDM_ACTION_MESSAGE, *start_datapack);
        auto_load_pack->appendCDataWrapperToArray(*start_message_datapack);
      }

      // finalize startup command array
      auto_load_pack->finalizeArrayForKey(UnitServerNodeDomainAndActionRPC::PARAM_CONTROL_UNIT_STARTUP_COMMAND);
    }
  }
}

CDWShrdPtr CUCommonUtility::getConfigurationToUse(const std::string                                                        &cu_uid,
                                                  const std::string                                                        &ds_attribute_name,
                                                  chaos::metadata_service::persistence::data_access::NodeDataAccess        *n_da,
                                                  chaos::metadata_service::persistence::data_access::ControlUnitDataAccess *cu_da,
                                                  PropertyGroup                                                            &control_unit_property_group) {
  int                          err = 0;
  ChaosSharedPtr<CDataWrapper> element_configuration;

  // fetch static information
  if ((err = cu_da->getInstanceDatasetAttributeConfiguration(cu_uid,
                                                             ds_attribute_name,
                                                             element_configuration))) {
    LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("Error loading the configuration for the the dataset's attribute: %1% for control unit: %2%") % ds_attribute_name % cu_uid));
  }

  // check if we need to do a restor at initialization time
  if (control_unit_property_group.hasProperty(chaos::ControlUnitPropertyKey::INIT_RESTORE_APPLY) &
          control_unit_property_group.getProperty(chaos::ControlUnitPropertyKey::INIT_RESTORE_APPLY).getPropertyValue().isValid() &&
      control_unit_property_group.getProperty(chaos::ControlUnitPropertyKey::INIT_RESTORE_APPLY).getPropertyValue().asBool()) {
    // in case of restore at init if we need to to a restor to a last valid dataset wi try to fetch it from database
    if (control_unit_property_group.hasProperty(chaos::ControlUnitPropertyKey::INIT_RESTORE_OPTION) &&
        control_unit_property_group.getProperty(chaos::ControlUnitPropertyKey::INIT_RESTORE_OPTION).getPropertyValue().isValid()) {
      if (control_unit_property_group.getProperty(chaos::ControlUnitPropertyKey::INIT_RESTORE_OPTION).getPropertyValue().asInt32() == chaos::ControlUnitPropertyKey::INIT_RESTORE_OPTION_TYPE_LAST_VALIDE) {
        // in this case we need only the value, but if the attribute has been set in static configuration all othe property are preserved
        // get last dataset
        CDWShrdPtr                 tmp_result;
        AbstractPersistenceDriver &obj_storage_drv = DriverPoolManager::getInstance()->getObjectStorageDrv();
        if (obj_storage_drv.getDataAccess<ObjectStorageDataAccess>()->getLastObject(cu_uid + chaos::DataPackPrefixID::INPUT_DATASET_POSTFIX, tmp_result) == 0) {
          if (tmp_result.get() != NULL) {
            ChaosStringSet all_keys;
            if (element_configuration.get() == NULL) {
              element_configuration.reset(new CDataWrapper());
            }
            // we have found last dataset
            ChaosSharedPtr<CDataWrapper> new_configuration(new CDataWrapper());
            element_configuration->getAllKey(all_keys);

            for (ChaosStringSetIterator it  = all_keys.begin(),
                                        end = all_keys.end();
                 it != end;
                 it++) {
              if (it->compare(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DEFAULT_VALUE) == 0) {
                tmp_result->copyKeyToNewKey(ds_attribute_name, ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DEFAULT_VALUE, *new_configuration);
              } else {
                element_configuration->copyKeyTo(*it, *new_configuration);
              }
            }
            // set new confiuration as element default configuration
            element_configuration = new_configuration;
          } else {
            // we have got problem to fetch last dataset so we trow exception
            throw CException(-1, "No dataset found on object storage", __PRETTY_FUNCTION__);
          }
        }
      }
    }
  }
  return element_configuration;
}
static int64_t                                    nu_cache_ts = 0;
static std::vector<ChaosSharedPtr<CDataWrapper> > data_services;
static CDataWrapper                               fillWrapperFromPersistenceParams(chaos::service_common::persistence::data_access::PersistenceDriverSetting &setting) {
  CDataWrapper persistence;
  if (setting.persistence_implementation.size()) {
    std::vector<std::string> pers_servers = setting.persistence_server_list;  // ChaosMetadataService::getInstance()->getGlobalConfigurationInstance()->getOption< std::vector< std::string> >(chaos::service_common::persistence::OPT_PERSITENCE_SERVER_ADDR_LIST);
    persistence.addStringValue(chaos::service_common::persistence::OPT_PERSITENCE_IMPL, setting.persistence_implementation);
    if (pers_servers.size()) {
      for (std::vector<std::string>::iterator i = pers_servers.begin(); i != pers_servers.end(); i++) {
        if (*i == "localhost" || (*i == "127.0.0.1")) {
          persistence.appendStringToArray(chaos::GlobalConfiguration::getInstance()->getLocalServerAddress());
        } else {
          persistence.appendStringToArray(*i);
        }
      }
      persistence.finalizeArrayForKey(chaos::service_common::persistence::OPT_PERSITENCE_SERVER_ADDR_LIST);
    }
    std::map<std::string, std::string> parameter = setting.persistence_kv_param_map;
    for (std::map<std::string, std::string>::iterator i = parameter.begin(); i != parameter.end(); i++) {
      persistence.appendStringToArray(i->first + ":" + i->second);
    }
    if (parameter.begin() != parameter.end()) {
      persistence.finalizeArrayForKey(chaos::service_common::persistence::OPT_PERSITENCE_KV_PARAMTER);
    }
  }
  return persistence;
}
void CUCommonUtility::addDataServicePack(ChaosUniquePtr<chaos::common::data::CDataWrapper> &result, chaos::metadata_service::persistence::data_access::DataServiceDataAccess *ds_da, unsigned numner_or_result) {
  int     err = 0;
  int64_t now = TimingUtil::getTimeStamp();

  const std::string &ha_zone_name = ChaosMetadataService::getInstance()->setting.ha_zone_name;

  if (now >= nu_cache_ts ||
      data_services.size() == 0) {
    data_services.clear();
    nu_cache_ts = now + chaos::common::constants::RefreshEndpointMSec;

    if ((err = ds_da->getBestNDataService(ha_zone_name,
                                          data_services,
                                          numner_or_result))) {
      throw CException(err, "Error fetching best available data service", __PRETTY_FUNCTION__);
    }
    // update cache on first call after ten seconds
  }
  std::string msgbroker = GlobalConfiguration::getInstance()->getOption<std::string>(InitOption::OPT_MSG_BROKER_SERVER);
  if (msgbroker.size()) {
    result->appendStringToArray(msgbroker);
    result->finalizeArrayForKey(chaos::DataServiceNodeDefinitionKey::DS_BROKER_ADDRESS_LIST);
    result->addInt64Value(chaos::DataServiceNodeDefinitionKey::DS_TIMESTAMP_UNCERTENTY, (uint64_t)ChaosMetadataService::timePrecisionMask);
  }

  if (DriverPoolManager::objectSetting.persistence_implementation.size()) {
    result->append(chaos::DataServiceNodeDefinitionKey::DS_STORAGE_SETTINGS, fillWrapperFromPersistenceParams(DriverPoolManager::objectSetting));
  }
  if (DriverPoolManager::persistentSetting.persistence_implementation.size()) {
    result->append(chaos::DataServiceNodeDefinitionKey::DS_ADMIN_SETTINGS, fillWrapperFromPersistenceParams(DriverPoolManager::persistentSetting));
  }
  if (DriverPoolManager::logSetting.persistence_implementation.size()) {
    result->append(chaos::DataServiceNodeDefinitionKey::DS_LOG_SETTINGS, fillWrapperFromPersistenceParams(DriverPoolManager::logSetting));
  }
  // std::vector<std::string> cache_server = GlobalConfiguration::getInstance()->getOption<std::vector<std::string> >(OPT_CACHE_SERVER_LIST);
  std::vector<std::string> cache_server = DriverPoolManager::cacheSetting.startup_chache_servers;
  if (cache_server.size()) {
    CDataWrapper cache;
    // cache.addStringValue(OPT_CACHE_DRIVER, GlobalConfiguration::getInstance()->getOption<std::string>(OPT_CACHE_DRIVER));
    cache.addStringValue(OPT_CACHE_DRIVER, DriverPoolManager::cacheSetting.cache_driver_impl);
    for (std::vector<std::string>::iterator i = cache_server.begin(); i != cache_server.end(); i++) {
      if (*i == "localhost" || (*i == "127.0.0.1")) {
        cache.appendStringToArray(GlobalConfiguration::getInstance()->getLocalServerAddress());
      } else {
        cache.appendStringToArray(*i);
      }
    }
    cache.finalizeArrayForKey(OPT_CACHE_SERVER_LIST);
    // std::vector<std::string> parameter = GlobalConfiguration::getInstance()->getOption<std::vector<std::string> >(OPT_CACHE_DRIVER_KVP);
    std::map<std::string, std::string> parameter = DriverPoolManager::cacheSetting.key_value_custom_param;
    for (std::map<std::string, std::string>::iterator i = parameter.begin(); i != parameter.end(); i++) {
      cache.appendStringToArray(i->first + ":" + i->second);
    }
    cache.finalizeArrayForKey(OPT_CACHE_DRIVER_KVP);
    result->append(chaos::DataServiceNodeDefinitionKey::DS_CACHE_SETTINGS, cache);
  }

  // constructs the result
  // result.reset(new CDataWrapper());
  if (data_services.size() == 0) {
    /// something wrong returning my self

    result->appendStringToArray(boost::str(boost::format("%1%|0") % NetworkBroker::getInstance()->getDirectIOUrl()));
    result->finalizeArrayForKey(chaos::DataServiceNodeDefinitionKey::DS_DIRECT_IO_FULL_ADDRESS_LIST);

    result->appendStringToArray(NetworkBroker::getInstance()->getRPCUrl());
    result->finalizeArrayForKey(chaos::NodeDefinitionKey::NODE_RPC_ADDR);
    CUCU_ERR << " no dataservice from DB, returning myself.:" << result->getJSONString();

    return;
  }
  if (data_services.size() > 0) {
    //  CUCU_DBG << CHAOS_FORMAT("Found %1% data services available", %data_services.size());
    BOOST_FOREACH (ChaosSharedPtr<CDataWrapper> ds_element, data_services) {
      if (ds_element->hasKey(chaos::NodeDefinitionKey::NODE_DIRECT_IO_ADDR) &&
          ds_element->hasKey(chaos::DataServiceNodeDefinitionKey::DS_DIRECT_IO_ENDPOINT)) {
        result->appendStringToArray(boost::str(boost::format("%1%|%2%") %
                                               ds_element->getStringValue(chaos::NodeDefinitionKey::NODE_DIRECT_IO_ADDR) %
                                               ds_element->getInt32Value(chaos::DataServiceNodeDefinitionKey::DS_DIRECT_IO_ENDPOINT)));
      }
    }
    result->finalizeArrayForKey(chaos::DataServiceNodeDefinitionKey::DS_DIRECT_IO_FULL_ADDRESS_LIST);

    // add rpc information for found nodes
    BOOST_FOREACH (ChaosSharedPtr<CDataWrapper> ds_element, data_services) {
      if (ds_element->hasKey(chaos::NodeDefinitionKey::NODE_RPC_ADDR)) {
        result->appendStringToArray(ds_element->getStringValue(chaos::NodeDefinitionKey::NODE_RPC_ADDR));
      }
    }
    result->finalizeArrayForKey(chaos::NodeDefinitionKey::NODE_RPC_ADDR);
  } else {
    // something is going wrong returning myself

    throw CException(err, "No best endpoint found", __PRETTY_FUNCTION__);
  }
  CUCU_DBG << "Configuration pack:" << result->getJSONString();
}
#if 0
void CUCommonUtility::addDataServicePack(const std::string& cu_uid,ChaosUniquePtr<chaos::common::data::CDataWrapper>& init_datapack,
                                                                                      chaos::metadata_service::persistence::data_access::ControlUnitDataAccess *cu_da,
                                                                                      chaos::metadata_service::persistence::data_access::DataServiceDataAccess *ds_da,unsigned results){

 std::vector<std::string> associated_ds;
 int err;
     const std::string& ha_zone_name = ChaosMetadataService::getInstance()->setting.ha_zone_name;

    //load the asosciated dataservice
    if((err = cu_da->getDataServiceAssociated(cu_uid,
                                              associated_ds))){
        LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("Error loading the associated data service for the control unit %1%") % cu_uid));
    }
    
    //check if we have found some
    if(associated_ds.size() == 0) {
        CUCU_DBG << "No dataservice has been found for control unit:" << cu_uid <<" so we need to get the best list them";
        //no we need to get tbest tree available cds to retun publishable address
        if((err = ds_da->getBestNDataService(ha_zone_name,
                                             associated_ds, 3))) {
            LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("Error fetching %2% best available data service for inititializing the control unit:%1%") % cu_uid % 3));
        }
    }
    
    
    //add endpoint(data service) where the control unit need to publish his dataset
    if(associated_ds.size()) {
        //add data service where published data
        BOOST_FOREACH(std::string ds_unique_id, associated_ds) {
            CDataWrapper *ds_description = NULL;
            if((err = ds_da->getDescription(ds_unique_id, &ds_description))) {
                LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("Error fetchin description for data service %1%") % ds_unique_id));
            } else if(ds_description == NULL) {
                CUCU_DBG << "No description foudn for data service:" << ds_unique_id;
            } else {
                ChaosUniquePtr<chaos::common::data::CDataWrapper> ds_object(ds_description);
                if(ds_object->hasKey(NodeDefinitionKey::NODE_DIRECT_IO_ADDR) &&
                   ds_object->hasKey(DataServiceNodeDefinitionKey::DS_DIRECT_IO_ENDPOINT)) {
                    //we can create the address
                    std::string direct_io_addr = boost::str(boost::format("%1%|%2%") %
                                                            ds_object->getStringValue(NodeDefinitionKey::NODE_DIRECT_IO_ADDR) %
                                                            ds_object->getInt32Value(DataServiceNodeDefinitionKey::DS_DIRECT_IO_ENDPOINT));
                    init_datapack->appendStringToArray(direct_io_addr);
                }
                
            }
            
        }
        init_datapack->finalizeArrayForKey(DataServiceNodeDefinitionKey::DS_DIRECT_IO_FULL_ADDRESS_LIST);
    }
   
}
#endif
ChaosUniquePtr<chaos::common::data::CDataWrapper> CUCommonUtility::initDataPack(const std::string     &cu_uid,
                                                                                NodeDataAccess        *n_da,
                                                                                ControlUnitDataAccess *cu_da,
                                                                                DataServiceDataAccess *ds_da) {
  int                                               err    = 0;
  int64_t                                           run_id = 0;
  CDataWrapper                                     *result = NULL;
  PropertyGroup                                     control_unit_property_group;
  ChaosUniquePtr<chaos::common::data::CDataWrapper> cu_base_description;
  ChaosUniquePtr<chaos::common::data::CDataWrapper> dataset_description;
  ChaosUniquePtr<chaos::common::data::CDataWrapper> init_datapack(new CDataWrapper());

  // set the control unique id in the init datapack
  init_datapack->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, cu_uid);

  // get default control unit node description
  if ((err = n_da->getNodeDescription(cu_uid, &result))) {
    LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("Error fetching the control unit node default description for unique id:%1% with error %2%") % cu_uid));
  }
  cu_base_description.reset(result);
  result = NULL;

  // copy rpc information in the init datapack
  if (init_datapack->hasKey(NodeDefinitionKey::NODE_RPC_ADDR)) {
    // CHECK_KEY_THROW_AND_LOG(cu_base_description, NodeDefinitionKey::NODE_RPC_ADDR, CUCU_ERR, -3, "No rpc addres in the control unit descirption")
    init_datapack->addStringValue(NodeDefinitionKey::NODE_RPC_ADDR, cu_base_description->getStringValue(NodeDefinitionKey::NODE_RPC_ADDR));
  }

  if (init_datapack->hasKey(NodeDefinitionKey::NODE_RPC_DOMAIN)) {
    // CHECK_KEY_THROW_AND_LOG(cu_base_description, NodeDefinitionKey::NODE_RPC_DOMAIN, CUCU_ERR, -3, "No rpc domain in the control unit descirption")
    init_datapack->addStringValue(NodeDefinitionKey::NODE_RPC_DOMAIN, cu_base_description->getStringValue(NodeDefinitionKey::NODE_RPC_DOMAIN));
  }

  if ((err = cu_da->getInstanceDescription(cu_uid,
                                           &result))) {
    LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("Error fetching the control unit instance description for control unit:%1%") % cu_uid));
  } else if (result == NULL) {
    LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("No instance found for control unit: %1%") % cu_uid));
  }
  // we have the configured instance
  ChaosUniquePtr<chaos::common::data::CDataWrapper> instance_description(result);
  result = NULL;

  // update run id
  // update the cotnrol unit id

  // better to return the timestamp is always growing, has a more information and faster
  /*  if((err = cu_da->getNextRunID(cu_uid,
                                  run_id))) {
        LOG_AND_TROW(CUCU_ERR, err, CHAOS_FORMAT("Error incrementig run id for control unit %1%", %cu_uid));
    }
    */
  if (instance_description.get()) {
    if (instance_description->hasKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_ALRM_MSK) && instance_description->isVectorValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_ALRM_MSK)) {
      CMultiTypeDataArrayWrapperSPtr v = instance_description->getVectorValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_ALRM_MSK);
      init_datapack->append(ControlUnitNodeDefinitionKey::CONTROL_UNIT_ALRM_MSK, v);
    }
    if (instance_description->hasKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_PROP) && instance_description->isVectorValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_PROP)) {
      CMultiTypeDataArrayWrapperSPtr v = instance_description->getVectorValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_PROP);
      init_datapack->append(ControlUnitNodeDefinitionKey::CONTROL_UNIT_PROP, v);
    }
  }
  run_id = chaos::common::utility::TimingUtil::getTimeStamp();
  // get the dataset of the control unit
  if ((err = cu_da->getDataset(cu_uid,
                               &result))) {
    LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("Error fetching dataset for control unit %1%") % cu_uid));
  } else if (result != NULL) {
    // we have the published dataset
    dataset_description.reset(result);

    // check for restore option
    if ((err = n_da->getPropertyGroup(PropertyTypeDefaultValues,
                                      cu_uid,
                                      chaos::ControlUnitPropertyKey::P_GROUP_NAME,
                                      control_unit_property_group))) {
      LOG_AND_TROW(CUCU_ERR, err, CHAOS_FORMAT("Error fetching defaults value for properti group %1% for control unit %2%", % chaos::ControlUnitPropertyKey::P_GROUP_NAME % cu_uid));
    }

    CMultiTypeDataArrayWrapperSPtr                    dataset_element_vec(dataset_description->getVectorValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION));
    ChaosUniquePtr<chaos::common::data::CDataWrapper> init_dataset(new CDataWrapper());
    for (int idx = 0; idx <
                      dataset_element_vec->size();
         idx++) {
      // get the dataset element
      ChaosUniquePtr<chaos::common::data::CDataWrapper> element(dataset_element_vec->getCDataWrapperElementAtIndex(idx));
      const std::string                                 ds_attribute_name = element->getStringValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_NAME);
      int32_t                                           direction         = element->getInt32Value(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DIRECTION);
      // get the dataset element setup
      if(element.get()==NULL) continue;
      
      if (direction == chaos::DataType::Input ||
          direction == chaos::DataType::Bidirectional) {
        CDWShrdPtr element_configuration = getConfigurationToUse(cu_uid,
                                                                 ds_attribute_name,
                                                                 n_da,
                                                                 cu_da,
                                                                 control_unit_property_group);
        if (element_configuration.get()) {
          //we can retrive the configured attribute
          ChaosUniquePtr<chaos::common::data::CDataWrapper> init_ds_attribute = mergeDatasetAttributeWithSetup(element.get(),
                                                                                                               element_configuration.get());
          if(init_ds_attribute.get()){
            init_dataset->appendCDataWrapperToArray(*init_ds_attribute.get());
          }
        } else {
          init_dataset->appendCDataWrapperToArray(*element.get());
        }
      } else {
        init_dataset->appendCDataWrapperToArray(*element.get());
      }
    }
    init_dataset->finalizeArrayForKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION);

    // add configurad dataset to the init datapack
    init_datapack->addCSDataValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION, *init_dataset.get());
  } else {
    // we have no dataset configure, in this case we send back only the configuration witho merge
    if ((err = cu_da->getInstanceDescription(cu_uid, &result))) {
      LOG_AND_TROW(CUCU_ERR, err, boost::str(boost::format("Error fetching the isntance for control unit %1%") % cu_uid));
    } else if (result != NULL) {
      instance_description.reset(result);
      ChaosUniquePtr<chaos::common::data::CDataWrapper> init_dataset(new CDataWrapper());
      if (instance_description->hasKey("attribute_value_descriptions") &&
          instance_description->isVectorValue("attribute_value_descriptions")) {
        // we have a configuration so we try to send it as dataset
        CMultiTypeDataArrayWrapperSPtr instance_description_array(result->getVectorValue("attribute_value_descriptions"));
        for (int aai = 0;
             aai < instance_description_array->size();
             aai++) {
          ChaosUniquePtr<chaos::common::data::CDataWrapper> attribute(instance_description_array->getCDataWrapperElementAtIndex(aai));

          init_dataset->appendCDataWrapperToArray(*attribute.get());
        }
      }
      init_dataset->finalizeArrayForKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION);

      // add configurad dataset to the init datapack
      init_datapack->addCSDataValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DESCRIPTION, *init_dataset.get());
    }
  }

  addDataServicePack(init_datapack, ds_da);

  PropertyGroupVectorSDWrapper pgu_default;
  if ((err = n_da->getProperty(PropertyTypeDefaultValues, cu_uid, pgu_default()))) {
    LOG_AND_TROW(CUCU_ERR, err, CHAOS_FORMAT("Error fetching property defaults for node %1%", % cu_uid));
  }

  pgu_default.serialization_key = "property";

  pgu_default.serialize()->copyAllTo(*init_datapack);

  init_datapack->addInt64Value(ControlUnitNodeDefinitionKey::CONTROL_UNIT_RUN_ID, run_id);
  // set the action type
  init_datapack->addInt32Value("action", (int32_t)0);
  return init_datapack;
}

void CUCommonUtility::mergeDefaultToProperty(PropertyGroupVector &src_group_vec,
                                             PropertyGroupVector &dst_group_vec) {
  for (PropertyGroupVectorIterator src_it  = src_group_vec.begin(),
                                   src_end = src_group_vec.end();
       src_it != src_end;
       src_it++) {
    for (PropertyGroupVectorIterator dst_it  = dst_group_vec.begin(),
                                     dst_end = src_group_vec.end();
         dst_it != dst_end;
         dst_it++) {
      if (src_it->getGroupName().compare(dst_it->getGroupName()) != 0) continue;

      dst_it->updatePropertiesValueFromSourceGroup(*src_it);
    }
  }
}

ChaosUniquePtr<chaos::common::data::CDataWrapper> CUCommonUtility::startDataPack(const std::string &cu_uid) {
  ChaosUniquePtr<chaos::common::data::CDataWrapper> result(new CDataWrapper());
  result->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, cu_uid);
  return result;
}

ChaosUniquePtr<chaos::common::data::CDataWrapper> CUCommonUtility::stopDataPack(const std::string &cu_uid) {
  ChaosUniquePtr<chaos::common::data::CDataWrapper> result(new CDataWrapper());
  result->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, cu_uid);
  return result;
}

ChaosUniquePtr<chaos::common::data::CDataWrapper> CUCommonUtility::deinitDataPack(const std::string &cu_uid) {
  ChaosUniquePtr<chaos::common::data::CDataWrapper> result(new CDataWrapper());
  result->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, cu_uid);
  return result;
}

ChaosUniquePtr<chaos::common::data::CDataWrapper> CUCommonUtility::mergeDatasetAttributeWithSetup(CDataWrapper *element_in_dataset,
                                                                                                  CDataWrapper *element_in_setup) {
  ChaosUniquePtr<chaos::common::data::CDataWrapper> result(new CDataWrapper());
  // move
  MOVE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_NAME, element_in_dataset, result)
  MOVE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DESCRIPTION, element_in_dataset, result)
  MOVE_INT32_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_TYPE, element_in_dataset, result)
  MOVE_INT32_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DIRECTION, element_in_dataset, result)
  MOVE_INT32_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_VALUE_MAX_SIZE, element_in_dataset, result)

  // update config value
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DEFAULT_VALUE, element_in_dataset, element_in_setup, result)
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_MAX_RANGE, element_in_dataset, element_in_setup, result)
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_MIN_RANGE, element_in_dataset, element_in_setup, result)
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_INCREMENT, element_in_dataset, element_in_setup, result)
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_UNIT, element_in_dataset, element_in_setup, result)
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_CONVFACT, element_in_dataset, element_in_setup, result)
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_OFFSET, element_in_dataset, element_in_setup, result)
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_WARN_THR, element_in_dataset, element_in_setup, result)
  MERGE_STRING_VALUE(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_ERROR_THR, element_in_dataset, element_in_setup, result)

  return result;
}
