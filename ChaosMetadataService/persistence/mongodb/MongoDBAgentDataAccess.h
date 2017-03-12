/*
 *	MongoDBAgentDataAccess.h
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 06/02/2017 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFramework_CB7E9739_B802_4DF0_BCFC_B98DAE7E0669_MongoDBAgentDataAccess_h
#define __CHAOSFramework_CB7E9739_B802_4DF0_BCFC_B98DAE7E0669_MongoDBAgentDataAccess_h

#include "../data_access/AgentDataAccess.h"
#include "MongoDBUtilityDataAccess.h"
#include "MongoDBNodeDataAccess.h"
#include <chaos/common/utility/ObjectInstancer.h>
#include <chaos_service_common/persistence/mongodb/MongoDBAccessor.h>

namespace chaos {
    namespace metadata_service {
        namespace persistence {
            namespace mongodb {
                //forward declaration
                class MongoDBPersistenceDriver;
                
                //! Data Access for manage the log
                /*!
                 */
                class MongoDBAgentDataAccess:
                public data_access::AgentDataAccess,
                protected service_common::persistence::mongodb::MongoDBAccessor {
                    friend class MongoDBPersistenceDriver;
                    
                    MongoDBUtilityDataAccess *utility_data_access = NULL;
                    MongoDBNodeDataAccess *node_data_access = NULL;
                    //return the query for a page
                    mongo::Query getNextPagedQuery(uint64_t last_sequence_before_this_page,
                                                   const std::string& search_string);
                    
                    mongo::Query getNextPagedQueryForInstance(uint64_t last_sequence_before_this_page,
                                                              const std::string& script_name,
                                                              const std::string& search_string);
                    
                protected:
                    MongoDBAgentDataAccess(const boost::shared_ptr<chaos::service_common::persistence::mongodb::MongoDBHAConnectionManager>& _connection);
                    ~MongoDBAgentDataAccess();
                public:
                    //! inherited by data_access::AgentDataAccess
                    int insertUpdateAgentDescription(chaos::common::data::CDataWrapper& agent_description);
                    
                    //!load full agent description
                    int loadAgentDescription(const std::string& agent_uid,
                                             const bool load_related_data,
                                             chaos::service_common::data::agent::AgentInstance& agent_description);
                    
                    //! inherited by data_access::AgentDataAccess
                    int getAgentForNode(const std::string& associated_node_uid,
                                        std::string& agent_uid);
                    
                    //! inherited by data_access::AgentDataAccess
                    int saveNodeAssociationForAgent(const std::string& agent_uid,
                                                    chaos::service_common::data::agent::AgentAssociation& node_association);
                    //! inherited by data_access::AgentDataAccess
                    int loadNodeAssociationForAgent(const std::string& agent_uid,
                                                    const std::string& associated_node_uid,
                                                    chaos::service_common::data::agent::AgentAssociation& node_association);
                    //! inherited by data_access::AgentDataAccess
                    int removeNodeAssociationForAgent(const std::string& agent_uid,
                                                      const std::string& associated_node_uid);
                    //! inherited by data_access::AgentDataAccess
                    int setNodeAssociationStatus(const std::string& agent_uid,
                                                 const chaos::service_common::data::agent::AgentAssociationStatus& status);
                    
                    //! inherited by data_access::AgentDataAccess
                    int getNodeListStatusForAgent(const std::string& agent_uid,
                                                  chaos::service_common::data::agent::VectorAgentAssociationStatus& node_status_vec);
                };
            }
        }
    }
}

#endif /* __CHAOSFramework_CB7E9739_B802_4DF0_BCFC_B98DAE7E0669_MongoDBAgentDataAccess_h */