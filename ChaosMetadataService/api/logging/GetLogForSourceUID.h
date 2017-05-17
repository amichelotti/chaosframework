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

#include "../AbstractApi.h"

namespace chaos {
    namespace metadata_service {
        namespace persistence {
            namespace data_access {
                struct LogEntry;
            }
        }
        namespace api {
            namespace logging {
                
                //! Return logging for a device uid
                /*!
                 The logging are paged so in query can be gave the last sequence id
                 */
                class GetLogForSourceUID:
                public AbstractApi {
                    inline ChaosUniquePtr<CDataWrapper> converEntry(persistence::data_access::LogEntry& log_entry);
                public:
                    GetLogForSourceUID();
                    ~GetLogForSourceUID();
                    chaos::common::data::CDataWrapper *execute(chaos::common::data::CDataWrapper *api_data,
                                                               bool& detach_data);
                };
            }
        }
    }
}

#endif /* __CHAOSFramework__GetLogForSourceUID_h */
