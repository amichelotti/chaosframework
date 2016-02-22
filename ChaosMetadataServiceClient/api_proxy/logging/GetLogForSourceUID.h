/*
 *	GetLogForSourceUID.h
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by Claudio Bisegni.
 *
 *    	Copyright 15/02/16 INFN, National Institute of Nuclear Physics
 *
 *    	Licensed under the Apache License, Version 2.0 (the "License");
 *    	you may not use this file except in compliance with the License.
 *    	You may obtain a copy of the License at
 *
 *    	http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */

#ifndef __CHAOSFramework__GetLogForSourceUID_h
#define __CHAOSFramework__GetLogForSourceUID_h

#include <ChaosMetadataServiceClient/api_proxy/ApiProxy.h>
#include <ChaosMetadataServiceClient/api_proxy/logging/logging_types.h>

#include <chaos/common/chaos_types.h>

namespace chaos {
    namespace metadata_service_client {
        namespace api_proxy {
            namespace logging {
                
                class GetLogForSourceUIDHelper {
                    LogEntryList log_entry_list;
                public:
                    GetLogForSourceUIDHelper(chaos::common::data::CDataWrapper *api_result);
                    ~GetLogForSourceUIDHelper();
                    size_t getLogEntryListSize();
                    const LogEntryList& getLogEntryList();
                };
                
                //! get log entries for a source
                class GetLogForSourceUID:
                public chaos::metadata_service_client::api_proxy::ApiProxy {
                    API_PROXY_CLASS(GetLogForSourceUID)
                protected:
                    API_PROXY_CD_DECLARATION(GetLogForSourceUID)
                public:
                    //! Add a new node in the domain
                    /*!
                     If the node_parent_path is an empty string(the default value) the ndoe became a root in the domain
                     \param source node that has emitted the log entries
                     \param domain if is not and empty string it will be used to filter domain in log entries
                     \param last_sequence_id is the id of the last returned entries in the past query
                     \param page_direction is the direction of the query to search forward(true) or backward(false)
                     */
                    ApiProxyResult execute(const std::string& source,
                                           const std::string& domain = std::string(),
                                           const uint64_t last_sequence_id = 0,
                                           const uint32_t page_length = 30,
                                           const bool page_direction = true);
                    
                    //! Add a new node in the domain
                    /*!
                     If the node_parent_path is an empty string(the default value) the ndoe became a root in the domain
                     \param source node that has emitted the log entries
                     \param domain a list of stirng, taht if is not and empty string it will be used to filter domain in log entries
                     \param last_sequence_id is the id of the last returned entries in the past query
                     \param page_direction is the direction of the query to search forward(true) or backward(false)
                     */
                    ApiProxyResult execute(const std::string& source,
                                           const LogDomainList& domain_list,
                                           const uint64_t last_sequence_id,
                                           const uint32_t page_length = 30,
                                           const bool page_direction = true);
                    
                    static std::auto_ptr<GetLogForSourceUIDHelper> getHelper(chaos::common::data::CDataWrapper *api_result);
                };
            }
        }
    }
}

#endif /* __CHAOSFramework__GetLogForSourceUID_h */
