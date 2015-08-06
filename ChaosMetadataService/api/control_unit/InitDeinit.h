/*
 *	InitDeinit.h
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
#ifndef __CHAOSFramework__InitDeinit__
#define __CHAOSFramework__InitDeinit__

#include "../AbstractApi.h"

namespace chaos {
    namespace metadata_service {
        namespace api {
            namespace control_unit {
                class InitDeinit:
                public AbstractApi {
                    void initialize(const std::string& cu_uid);
                    void deinitialize(const std::string& cu_uid);
                    boost::shared_ptr<CDataWrapper>     mergeDatasetAttributeWithSetup(boost::shared_ptr<CDataWrapper> element_in_dataset,
                                                                                       boost::shared_ptr<CDataWrapper> element_in_setup);
                public:
                    InitDeinit();
                    ~InitDeinit();
                    chaos::common::data::CDataWrapper *execute(chaos::common::data::CDataWrapper *api_data,
                                                               bool& detach_data) throw(chaos::CException);
                };
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__InitDeinit__) */
