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
#include <chaos/common/global.h>
#include <chaos_service_common/persistence/data_access/AbstractPersistenceDriver.h>

using namespace chaos::service_common::persistence::data_access;

#define APD_INFO INFO_LOG(AbstractPersistenceDriver)
#define APD_DBG DBG_LOG(AbstractPersistenceDriver)
#define APD_ERR ERR_LOG(AbstractPersistenceDriver)

int PersistenceDriverSetting::init(const chaos::common::data::CDataWrapper& cs) {
  if (cs.hasKey(OPT_PERSITENCE_IMPL)) {
    persistence_implementation = cs.getStringValue(OPT_PERSITENCE_IMPL);
    APD_DBG << "persistent implementation:" << persistence_implementation;
  }else {
    APD_ERR << " NO implementation given:" << cs.getJSONString();
    return -1;
  }
  if (cs.hasKey(OPT_PERSITENCE_SERVER_ADDR_LIST) && cs.isVectorValue(OPT_PERSITENCE_SERVER_ADDR_LIST)) {
    chaos::common::data::CMultiTypeDataArrayWrapperSPtr v = cs.getVectorValue(OPT_PERSITENCE_SERVER_ADDR_LIST);
    if (v.get() && v->size()) {
      persistence_server_list = *v;
      APD_DBG << " startup servers:" << persistence_server_list.size() << " " << persistence_server_list[0];
    } else {
      APD_ERR << " NO startup server list given:" << cs.getJSONString();

      return -2;
    }
  }

  if (cs.hasKey(OPT_PERSITENCE_KV_PARAMTER) && cs.isVectorValue(OPT_PERSITENCE_KV_PARAMTER)) {
    chaos::common::data::CMultiTypeDataArrayWrapperSPtr v = cs.getVectorValue(OPT_PERSITENCE_KV_PARAMTER);
    if (v.get()) {
      std::vector<std::string> a = *v;
      chaos::GlobalConfiguration::fillKVParameter(persistence_kv_param_map, a, "[a-zA-Z0-9/_-]+:[a-zA-Z0-9/_-]+");
      for (std::map<std::string, std::string>::iterator i = persistence_kv_param_map.begin(); i != persistence_kv_param_map.end(); i++) {
        APD_DBG << i->first << "=" << i->second;
      }
    }
  }
  return 0;

}
AbstractPersistenceDriver::AbstractPersistenceDriver(const std::string& name)
    : NamedService(name) {
}

AbstractPersistenceDriver::~AbstractPersistenceDriver() {
}

// Initialize the driver
void AbstractPersistenceDriver::init(void* init_data) {
  if (init_data) {
    settings = *(PersistenceDriverSetting*)init_data;
  }
}

// deinitialize the driver
void AbstractPersistenceDriver::deinit() {
  for (MapDAIterator i = map_data_access.begin();
       i != map_data_access.end();
       i++) {
    APD_INFO << "Delete data access:" << i->first;
    deleteDataAccess((AbstractDataAccess*)i->second);
  }
}
