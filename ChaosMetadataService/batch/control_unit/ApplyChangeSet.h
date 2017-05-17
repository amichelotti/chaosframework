/*
 *	IDSTControlUnitBatchCommand.h
 *	!CHAOS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2015 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFramework__ApplyChangeSet__
#define __CHAOSFramework__ApplyChangeSet__

#include "../mds_service_batch.h"

namespace chaos {
    namespace metadata_service{
        namespace batch {
            class MDSBatchExcecutor;
            namespace control_unit {
                
                //!Batch command for apply a change set to a control unit
                class ApplyChangeSet:
                public metadata_service::batch::MDSBatchCommand {
                    DECLARE_MDS_COMMAND_ALIAS
                    uint32_t retry_number;
                    
                    CNetworkAddress * control_unit_address;
                    chaos::common::data::CDataWrapper *message;
                    ChaosUniquePtr<RequestInfo> request;
                    
                    std::string cu_id;
                public:
                    ApplyChangeSet();
                    ~ApplyChangeSet();
                protected:
                    // inherited method
                    void setHandler(chaos_data::CDataWrapper *data);
                    
                    // inherited method
                    void acquireHandler();
                    
                    // inherited method
                    void ccHandler();
                    
                    // inherited method
                    bool timeoutHandler();
                };
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__ApplyChangeSet__) */
