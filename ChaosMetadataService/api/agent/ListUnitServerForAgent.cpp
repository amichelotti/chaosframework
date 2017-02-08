/*
 *	ListUnitServerForAgent.cpp
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 08/02/2017 INFN, National Institute of Nuclear Physics
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

#include "ListUnitServerForAgent.h"

#include <chaos/common/data/structured/Lists.h>

using namespace chaos::metadata_service::api::agent;

#define INFO INFO_LOG(ListUnitServerForAgent)
#define ERR  DBG_LOG(ListUnitServerForAgent)
#define DBG  ERR_LOG(ListUnitServerForAgent)

using namespace chaos::common::data;
using namespace chaos::common::data::structured;
using namespace chaos::metadata_service::api::agent;
using namespace chaos::metadata_service::persistence::data_access;

ListUnitServerForAgent::ListUnitServerForAgent():
AbstractApi("listUnitServerForAgent"){
}

ListUnitServerForAgent::~ListUnitServerForAgent() {
}

CDataWrapper *ListUnitServerForAgent::execute(CDataWrapper *api_data, bool& detach_data) {
    //check for mandatory attributes
    CHECK_CDW_THROW_AND_LOG(api_data, ERR, -1, "No parameter found");
    CHECK_KEY_THROW_AND_LOG(api_data, NodeDefinitionKey::NODE_UNIQUE_ID, ERR, -2, CHAOS_FORMAT("The key %1% is mandatory", %NodeDefinitionKey::NODE_UNIQUE_ID));
    CHAOS_LASSERT_EXCEPTION(api_data->isStringValue(NodeDefinitionKey::NODE_UNIQUE_ID), ERR, -3, CHAOS_FORMAT("The key %1% need to be a string", %NodeDefinitionKey::NODE_UNIQUE_ID));

    //we can rpocessd
    GET_DATA_ACCESS(AgentDataAccess, a_da, -4);
    
    int err = 0;
    ChaosStringVectorSDWrapper association_list_sd_wrapper;
    association_list_sd_wrapper.serialization_key = "association_list";
    const std::string agent_uid = api_data->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID);

    if((err = a_da->getNodeListForAgent(agent_uid, association_list_sd_wrapper()))) {
        LOG_AND_TROW(ERR, -5, "Error creaating new log entry");
    }
    return association_list_sd_wrapper.serialize().release();
}
