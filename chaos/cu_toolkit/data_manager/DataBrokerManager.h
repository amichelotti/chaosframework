/*
 *	DataBrokerManager.h
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 25/07/16 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFramework_E89CBC0E_6B48_465C_8200_3ABE5701D169_DataBrokerManager_h
#define __CHAOSFramework_E89CBC0E_6B48_465C_8200_3ABE5701D169_DataBrokerManager_h

#include <chaos/common/utility/Singleton.h>

namespace chaos{
    namespace cu {
        namespace data_manager {
            
            //!forward declaration
            class DataBroker;
            
            //! data broker is the managemnt center for all dataset
            /*!
             every instance of data broker will be used by a control unit
             to define, publish and update the dataset versus other nodes(CDS 
             or other CU.
             */
            class DataBrokerManager:
            public chaos::common::utility::Singleton<DataBrokerManager> {
                friend class chaos::common::utility::Singleton<DataBrokerManager>;
                DataBrokerManager();
                ~DataBrokerManager();
            public:
                
                DataBroker *newBrokerInstance();
                void releaseBrokerInstance(DataBroker *broker);
                
            };
            
        }
    }
}

#endif /* __CHAOSFramework_E89CBC0E_6B48_465C_8200_3ABE5701D169_DataBrokerManager_h */