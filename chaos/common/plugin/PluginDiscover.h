/*
 *	PluginDiscover.h
 *	!CHAOS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2013 INFN, National Institute of Nuclear Physics
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

#ifndef CHAOSFramework_PluginDiscover_h
#define CHAOSFramework_PluginDiscover_h

#include <map>
#include <vector>
#include <iostream>

namespace chaos {
    namespace common{
        namespace plugin {

            //! Postifix of the allocator
            /*!
             Define the postifx of the allocator "c" function, exported by the ddl.
             */
#define PLUGIN_DISCOVER_POSTFIX   "_discover"
            
            class PluginDiscover {
                friend PluginDiscover* getInspector();
                
                std::vector<std::string> names;

            public:
                PluginDiscover();
                ~PluginDiscover();
                
                void addName(const char *name);
                
                size_t getNamesSize();
                
                const char * const getNameForIndex(size_t idx);
            };
        }
    }
}

#endif

