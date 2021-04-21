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

#ifndef __CHAOSFramework__DriverPoolManager__
#define __CHAOSFramework__DriverPoolManager__
#include <chaos/common/pool/ResourcePool.h>
#include <chaos/common/caching_system/CacheDriver.h>
#include <chaos_service_common/persistence/data_access/AbstractPersistenceDriver.h>

#include <chaos/common/utility/Singleton.h>
#include <chaos/common/utility/InizializableService.h>

#include <chaos_service_common/persistence/data_access/AbstractPersistenceDriver.h>

namespace chaos{
    namespace service_common {
        typedef chaos::common::pool::ResourcePool<chaos::common::cache_system::CacheDriver> CachePool;
        typedef CachePool::ResourcePoolHelper CachePoolHelper;
        typedef CachePool::ResourceSlot CachePoolSlot;
        
        
        //!base singleto class for driver pool system
        class DriverPoolManager:
        public chaos::common::utility::Singleton<DriverPoolManager>,
        public chaos::common::utility::InizializableService {
            friend class chaos::common::utility::Singleton<DriverPoolManager>;
            chaos::common::utility::InizializableServiceContainer<chaos::service_common::persistence::data_access::AbstractPersistenceDriver> persistence_driver;
            chaos::common::utility::InizializableServiceContainer<chaos::service_common::persistence::data_access::AbstractPersistenceDriver> storage_driver;
            chaos::common::utility::InizializableServiceContainer<chaos::service_common::persistence::data_access::AbstractPersistenceDriver> log_driver;

            chaos::common::utility::InizializableServiceContainer<chaos::common::cache_system::CacheDriver> cache_driver;

            DriverPoolManager();
            ~DriverPoolManager();
        protected:
            //timer handler
            void init(void *init_data);
            void deinit();
            
        public:
            static chaos::service_common::persistence::data_access::PersistenceDriverSetting persistentSetting;
            static chaos::service_common::persistence::data_access::PersistenceDriverSetting objectSetting;
            static chaos::service_common::persistence::data_access::PersistenceDriverSetting logSetting;
            static chaos::common::cache_system::CacheDriverSetting cacheSetting;

            chaos::common::cache_system::CacheDriver& getCacheDrv();
            chaos::common::cache_system::CacheDriver* getCacheDrvPtr();

            
            chaos::service_common::persistence::data_access::AbstractPersistenceDriver& getPersistenceDrv();
            chaos::service_common::persistence::data_access::AbstractPersistenceDriver* getPersistenceDrvPtr();

            template<typename T>
            T* getPersistenceDataAccess() {
                if(persistence_driver.get() == NULL) throw CException(-1, "No Persistence Driver Found", __PRETTY_FUNCTION__);
                return persistence_driver->getDataAccess<T>();
            }
            
            chaos::service_common::persistence::data_access::AbstractPersistenceDriver& getObjectStorageDrv();
            template<typename T>
            T* getStorageDataAccess() {
                if(storage_driver.get() == NULL) throw CException(-1, "No Storage Driver Found", __PRETTY_FUNCTION__);
                return storage_driver->getDataAccess<T>();
            }

            chaos::service_common::persistence::data_access::AbstractPersistenceDriver& getLogDrv();
            template<typename T>
            T* getLogDataAccess() {
                if(log_driver.get() == NULL) throw CException(-1, "No Logging Driver Found", __PRETTY_FUNCTION__);
                return log_driver->getDataAccess<T>();
            }
        };
    }
}

#endif /* defined(__CHAOSFramework__DriverPoolManager__) */
