/*
 *	ScriptDataAccess.h
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 25/05/16 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFramework__D7C2C2C_991D_4C87_996D_74166C62D9A4_ScriptDataAccess_h
#define __CHAOSFramework__D7C2C2C_991D_4C87_996D_74166C62D9A4_ScriptDataAccess_h

#include "../persistence.h"

#include <chaos/common/chaos_types.h>

#include <chaos_service_common/data/script/Script.h>

namespace chaos {
    namespace metadata_service {
        namespace persistence {
            namespace data_access {
                //!data access for script management
                class ScriptDataAccess:
                public chaos::service_common::persistence::data_access::AbstractDataAccess {
                    
                public:
                    DECLARE_DA_NAME
                    
                    //! default constructor
                    ScriptDataAccess();
                    
                    //!default destructor
                    ~ScriptDataAccess();
                    
                    //! Insert a new script in the database
                    /*!
                     \param Script that describe the script entry
                     */
                    virtual int insertNewScript(chaos::service_common::data::script::Script& new_Script) = 0;
                    
                    //! udate the script content
                    /*!
                     the script is identified using the base description
                     */
                    virtual int updateScriptContent(chaos::service_common::data::script::ScriptBaseDescription& script_identification,
                                                    const std::string script_content) = 0;
                    
                    //!Perform a search on script entries
                    /*!
                     perform a simple search on node filtering on type
                     \param script_list the found elemento for current page
                     \param search_string is the search string
                     \param start_sequence_id is identified the sequence after wich we need to search
                     \param page_length is the maximum number of the element to return
                     */
                    virtual int searchScript(chaos::service_common::data::script::ScriptList& script_list,
                                             const std::string& search_string,
                                             uint64_t start_sequence_id,
                                             uint32_t page_length) = 0;
                    
                };
            }
        }
    }
}

#endif /* __CHAOSFramework__D7C2C2C_991D_4C87_996D_74166C62D9A4_ScriptDataAccess_h */
