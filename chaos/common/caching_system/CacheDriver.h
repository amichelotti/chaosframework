/*
 * Copyright 2012, 2021 INFN
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

#ifndef __CHAOSFramework__CacheDriver__
#define __CHAOSFramework__CacheDriver__

//#include "../dataservice_global.h"

#include <stdint.h>
#include <string>

#include <chaos/common/chaos_types.h>
#include <chaos/common/data/Buffer.hpp>
#include <chaos/common/utility/NamedService.h>
#include <chaos/common/utility/InizializableService.h>
#include "cache_system_types.h"

namespace chaos {
    namespace common {
        namespace cache_system {
            
            typedef chaos::common::data::BufferSPtr CacheData;
            
            CHAOS_DEFINE_MAP_FOR_TYPE(std::string, CacheData, MultiCacheData);
            
                //! Abstraction of the chache driver
            /*!
             This class represent the abstraction of the 
             work to do on cache. Cache system is to be intended as global
             to all CacheDriver instance.
             */
            class CacheDriver :
            public chaos::common::utility::NamedService,
			public chaos::common::utility::InizializableService {
			protected:
				CacheDriver(std::string alias);
            public:
                CacheDriverSetting cache_settings;
                static std::map<std::string,std::pair<int64_t,chaos::common::data::CDWShrdPtr> > first_level_cache;
                static std::map<std::string,int32_t> enable_cache_for_ms;
                /**
                 * @brief enable cache for key
                 * 
                 * 
                 * @param validity_ms =0 means disable
                 */
                void enableCache(const std::string&key,int32_t validity_ms);
				virtual ~CacheDriver();
				
                virtual int putData(const std::string& key,
									CacheData data_to_store) = 0;

                virtual int deleteData(const std::string& key) = 0;

                
                virtual int getData(const std::string& key,
									CacheData& cached_data) = 0;
                
                virtual int getData(const ChaosStringVector&    keys,
                                    MultiCacheData&             multi_data) = 0;

                virtual int addServer(const std::string& server_desc) = 0;
                
                virtual int removeServer(const std::string& server_desc) = 0;
				
				//! init
				/*!
				 Need a point to a structure DBDriverSetting for the setting
				 */
				void init(void *init_data);
				
				//!deinit
				void deinit();
                chaos::common::data::CDWShrdPtr getData(const std::string& key);
                std::vector<chaos::common::data::CDWShrdPtr> getData(const ChaosStringVector&    keys);
                                    
            };
        }
    }
}

#endif /* defined(__CHAOSFramework__CacheDriver__) */
