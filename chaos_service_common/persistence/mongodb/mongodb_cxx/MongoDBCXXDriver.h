/*
 * Copyright 2012, 18/06/2018 INFN
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

#ifndef __CHAOSFramework__BaseMongoDBDiver__h
#define __CHAOSFramework__BaseMongoDBDiver__h

#if defined(__APPLE__)
#include <mongocxx/v_noabi/mongocxx/pool.hpp>
#include <mongocxx/v_noabi/mongocxx/instance.hpp>
#else
#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>
#endif

#include <chaos/common/chaos_types.h>
namespace chaos {
    namespace service_common {
        namespace persistence {
            namespace mongodb {
                namespace mongodb_cxx {
                    class BaseMongoDBDiver {
                        static mongocxx::instance       drv_instance;
                        ChaosSharedPtr<mongocxx::pool>  pool_shared_ptr;
                    protected:
                        void initPool(const ChaosStringVector& url_list,
                                      const std::string& user,
                                      const std::string& password,
                                      const std::string& database);
                        mongocxx::pool& getPool();
                        ChaosSharedPtr<mongocxx::pool> getSharedPool();
                    public:
                        BaseMongoDBDiver();
                        virtual ~BaseMongoDBDiver();
                    };
                }
            }
        }
    }
}

#endif /* __CHAOSFramework__BaseMongoDBDiver__h */
