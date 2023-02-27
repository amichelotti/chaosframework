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

#ifndef __CHAOSFramework__DeleteLog_h
#define __CHAOSFramework__DeleteLog_h


#include <chaos_metadata_service_client/api_proxy/ApiProxy.h>
#include <chaos_metadata_service_client/api_proxy/logging/logging_types.h>
#include <chaos_metadata_service_client/api_proxy/logging/GetLogForSourceUID.h>

#include <chaos/common/chaos_types.h>

namespace chaos {
    namespace metadata_service_client {
        namespace api_proxy {
            namespace logging {
                
                
                //! get log entries for a source
                class DeleteLog:
                public chaos::metadata_service_client::api_proxy::ApiProxy {
                    API_PROXY_CLASS(DeleteLog)
                protected:
                    API_PROXY_CD_DECLARATION(DeleteLog)
                public:

                    //! Perform an advanced search specifind also the range of ts
                    /*!
                     \param search_string is the string that is search with a 'like' procedure in all fields
                     \param domain is the lis tof domain to wich need to belog the result entry
                     \param start_ts if > 0 is used has start timestamp
                     \param end_ts if > 0 is used as end timestamp
                     \param last_sequence_id is the id of the last returned entries in the past query
                     \param page_length is the length of the returned element
                     */
                    ApiProxyResult execute(const std::string& uid,const std::string& dom,
                                           const uint64_t start_ts);
                };
            }
        }
    }
}

#endif /* __CHAOSFramework__DeleteLog_h */
