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

#ifndef __CHAOSFramework__IODirectIOPSMsgDriver__
#define __CHAOSFramework__IODirectIOPSMsgDriver__

#include <set>
#include <map>
#include <string>

#include "IODirectIODriver.h"
#include <chaos/common/message/MessagePSProducer.h>
#include <chaos/common/message/MessagePSDriver.h>
#include <chaos/common/caching_system/CacheDriver.h>

namespace chaos_data = chaos::common::data;
namespace chaos_utility = chaos::common::utility;
namespace chaos_direct_io = chaos::common::direct_io;
namespace chaos_dio_channel = chaos::common::direct_io::channel;

namespace chaos{
    namespace common {
        namespace io {
            
            
            /*!
             */
            DECLARE_CLASS_FACTORY(IODirectIOPSMsgDriver, IODirectIODriver)
            {
                REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(IODirectIOPSMsgDriver)
               

            protected:
              std::string msgbroker;
              std::string msgbrokerdrv;
              chaos::common::message::producer_uptr_t prod;
              chaos::common::message::consumer_uptr_t cons;
              chaos::common::message::MDSMessageChannel *mds_channel;
              void defaultHandler( chaos::common::message::ele_t& data);
               ChaosMutex hmutex;
               std::map<std::string,chaos::common::message::msgHandler> handler_map;
                ChaosSharedMutex                    map_query_future_mutex;
                std::map<std::string, QueryCursor*>	map_query_future;
                chaos::common::utility::InizializableServiceContainer<chaos::common::cache_system::CacheDriver> cache_driver;
                chaos::common::cache_system::CacheDriverSetting setpar;
                int have_direct_cache;
                void tryCacheInit();
            public:
                
                IODirectIOPSMsgDriver(const std::string& alias);
                virtual ~IODirectIOPSMsgDriver();
                
                
                /*
                 * Init method
                 */
                void init(void *init_parameter);
                
                /*
                 * Deinit method
                 */
                void deinit();
                
                int storeHealthData(const std::string& key,
                                      chaos_data::CDWShrdPtr data_to_store,
                                      DataServiceNodeDefinitionType::DSStorageType storage_type,
                                      const ChaosStringSet& tag_set= ChaosStringSet());
                /*
                 * storeRawData
                 */
                int storeData(const std::string& key,
                               chaos_data::CDWShrdPtr& data_to_store,
                               DataServiceNodeDefinitionType::DSStorageType storage_type,
                               const ChaosStringSet& tag_set = ChaosStringSet());
                /**
                 *
                 */
                int removeData(const std::string& key,
                               uint64_t start_ts,
                               uint64_t end_ts);
                 /**
                 *
                 */
                int retriveMultipleData(const ChaosStringVector& key,
                                        chaos::common::data::VectorCDWShrdPtr& result);
                
                /*
                 * retrieveRawData
                 */
                chaos::common::data::CDWUniquePtr retrieveData(const std::string& key);
                
                //! restore from a tag a dataset associated to a key
                int loadDatasetTypeFromSnapshotTag(const std::string& restore_point_tag_name,
                                                   const std::string& key,
                                                   uint32_t dataset_type,
                                                   chaos_data::CDWShrdPtr& cdw_shrd_ptr);
                
                /**
                 *
                 */
                

                QueryCursor* performQuery(const std::string&    key,
                                          uint64_t              start_ts,
                                          uint64_t              end_ts,
                                          const ChaosStringSet& meta_tags = ChaosStringSet(),
                                          const ChaosStringSet& projection_keys = ChaosStringSet(),
                                          uint32_t              page      = DEFAULT_PAGE_LEN);

                QueryCursor* performQuery(const std::string&    key,
                                          uint64_t              start_ts,
                                          uint64_t              end_ts,
                                          uint64_t              sequid,
                                          uint64_t              runid,
                                          const ChaosStringSet& meta_tags = ChaosStringSet(),
                                          const ChaosStringSet& projection_keys = ChaosStringSet(),
                                          uint32_t              page      = DEFAULT_PAGE_LEN);


               chaos::common::data::CDataWrapper* updateConfiguration(chaos::common::data::CDataWrapper* newConfigration);
                int subscribe(const std::string&key);
  
                int addHandler(const std::string&key,chaos::common::message::msgHandler cb);
                int addHandler(chaos::common::message::msgHandler cb);


            };
        }
    }
}


#endif /* defined(__CHAOSFramework__IODirectIOPSMsgDriver__) */
