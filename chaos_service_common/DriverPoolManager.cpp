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

#include "DriverPoolManager.h"

#if CHAOS_PROMETHEUS
#include "cache_system/CacheDriverMetricCollector.h"
#endif

#include <chaos/common/global.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <limits>
#define DP_LOG_INFO INFO_LOG(DriverPoolManager)
#define DP_LOG_DBG DBG_LOG(DriverPoolManager)
#define DP_LOG_ERR ERR_LOG(DriverPoolManager)

using namespace chaos::service_common;
using namespace chaos::common::utility;
using namespace chaos::common::async_central;
using namespace chaos::service_common::persistence::data_access;
//using namespace chaos::metadata_service::object_storage::abstraction;

chaos::service_common::persistence::data_access::PersistenceDriverSetting DriverPoolManager::persistentSetting;
chaos::service_common::persistence::data_access::PersistenceDriverSetting DriverPoolManager::objectSetting;
chaos::service_common::persistence::data_access::PersistenceDriverSetting DriverPoolManager::logSetting;
chaos::common::cache_system::CacheDriverSetting                           DriverPoolManager::cacheSetting;
//-------------------------------------------global pool---------------------------------------
DriverPoolManager::DriverPoolManager() {}

DriverPoolManager::~DriverPoolManager() {}

void DriverPoolManager::init(void* init_data) {
  //init cache pool
  //InizializableService::initImplementation(cache_pool, NULL, "CacheDriverPool", __PRETTY_FUNCTION__);
  const std::string cache_impl_name = cacheSetting.cache_driver_impl + "CacheDriver";
  if (cacheSetting.cache_driver_impl.size()) {
#if CHAOS_PROMETHEUS
    cache_driver.reset(new CacheDriverMetricCollector(ObjectFactoryRegister<chaos::common::cache_system::CacheDriver>::getInstance()->getNewInstanceByName(cache_impl_name)), cache_impl_name);
#else
    cache_driver.reset(ObjectFactoryRegister<chaos::common::cache_system::CacheDriver>::getInstance()->getNewInstanceByName(cache_impl_name),
                       cache_impl_name);
#endif

    if (cache_driver.get() == NULL) throw chaos::CException(-1, CHAOS_FORMAT("No %1% Cache Driver found", % cache_impl_name), __PRETTY_FUNCTION__);
    try {
      cache_driver.init((void*)&cacheSetting, __PRETTY_FUNCTION__);
    } catch (CException& e) {
      cache_driver.reset(NULL,cache_impl_name);
      throw e;
    } catch (...) {
      DP_LOG_ERR << " Undefined exception catchd during initialization of cache driver";

      cache_driver.reset(NULL,cache_impl_name);
    }
  }
  //init dirver instace
  const std::string persistence_impl_name = persistentSetting.persistence_implementation + "PersistenceDriver";
  if (persistentSetting.persistence_implementation.size()) {
    persistence_driver.reset(ObjectFactoryRegister<chaos::service_common::persistence::data_access::AbstractPersistenceDriver>::getInstance()->getNewInstanceByName(persistence_impl_name),
                             persistence_impl_name);
    if (persistence_driver.get() == NULL) throw chaos::CException(-1, CHAOS_FORMAT("No %1% Persistence Driver found", % persistence_impl_name), __PRETTY_FUNCTION__);
    try {
      persistence_driver.init((void*)&persistentSetting, __PRETTY_FUNCTION__);
    } catch (CException& e) {
      persistence_driver.reset(NULL,persistence_impl_name);
      throw e;
    } catch (...) {
      DP_LOG_ERR << " Undefined exception catchd during initialization of persistent driver";

      persistence_driver.reset(NULL,persistence_impl_name);
    }
  }
  const std::string storage_impl_name = objectSetting.persistence_implementation + "ObjectStorageDriver";
  if (objectSetting.persistence_implementation.size()) {
    storage_driver.reset(ObjectFactoryRegister<chaos::service_common::persistence::data_access::AbstractPersistenceDriver>::getInstance()->getNewInstanceByName(storage_impl_name),
                         storage_impl_name);
    if (storage_driver.get() == NULL) throw chaos::CException(-1, CHAOS_FORMAT("No %1% Object Storage Driver found", % storage_impl_name), __PRETTY_FUNCTION__);
    try {
      storage_driver.init((void*)&objectSetting, __PRETTY_FUNCTION__);
    } catch (CException& e) {
      storage_driver.reset(NULL,storage_impl_name);
      throw e;
    } catch (...) {
      DP_LOG_ERR << " Undefined exception catchd during initialization of storage driver";

      storage_driver.reset(NULL,storage_impl_name);
    }
  }
  const std::string log_impl_name = logSetting.persistence_implementation + "LogStorageDriver";
  if (logSetting.persistence_implementation.size()) {
    log_driver.reset(ObjectFactoryRegister<chaos::service_common::persistence::data_access::AbstractPersistenceDriver>::getInstance()->getNewInstanceByName(log_impl_name),
                     log_impl_name);
    if (log_driver.get() == NULL) {
      DP_LOG_INFO << " Log driver not defined";
    } else {
      try {
        log_driver.init((void*)&logSetting, __PRETTY_FUNCTION__);
      } catch (CException& ex) {
        log_driver.reset(NULL,log_impl_name);
        DECODE_CHAOS_EXCEPTION(ex)

      } catch (...) {
        DP_LOG_ERR << " Undefined exception catchd during initialization of LOG driver";
        log_driver.reset(NULL,log_impl_name);
      }
    }
  }
}

void DriverPoolManager::deinit() {
  storage_driver.deinit(__PRETTY_FUNCTION__);
  persistence_driver.deinit(__PRETTY_FUNCTION__);
  cache_driver.deinit(__PRETTY_FUNCTION__);
  if (log_driver.get()) {
    log_driver.deinit(__PRETTY_FUNCTION__);
  }
}

//--------------cach driver pool method--------------
chaos::common::cache_system::CacheDriver& DriverPoolManager::getCacheDrv() {
  return *cache_driver;
}

AbstractPersistenceDriver& DriverPoolManager::getPersistenceDrv() {
  return *persistence_driver;
}
chaos::common::cache_system::CacheDriver* DriverPoolManager::getCacheDrvPtr() {
  return cache_driver.get();
}

AbstractPersistenceDriver* DriverPoolManager::getPersistenceDrvPtr() {
  return persistence_driver.get();
}
chaos::service_common::persistence::data_access::AbstractPersistenceDriver* DriverPoolManager::getObjectStorageDrvPtr() {
  return storage_driver.get();
}

AbstractPersistenceDriver& DriverPoolManager::getObjectStorageDrv() {
  return *storage_driver;
}
AbstractPersistenceDriver& DriverPoolManager::getLogDrv() {
  return *log_driver;
}
AbstractPersistenceDriver* DriverPoolManager::getLogDrvPtr() {
  return log_driver.get();
}