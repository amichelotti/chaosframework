/*
 *	UtilityDataAccess.h
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
#ifndef __CHAOSFramework__UtilityDataAccess__
#define __CHAOSFramework__UtilityDataAccess__

#include "../persistence.h"

namespace chaos {
    namespace metadata_service {
        namespace persistence {
            namespace data_access {
                
                class UtilityDataAccess:
                public chaos::service_common::persistence::data_access::AbstractDataAccess{
                public:
                    DECLARE_DA_NAME

                    //! default constructor
                    UtilityDataAccess();
                    
                    //!default destructor
                    ~UtilityDataAccess();

                    virtual int getNextSequenceValue(const std::string& sequence_name, uint64_t& next_value) = 0;
                };
                
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__UtilityDataAccess__) */
