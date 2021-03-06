/*
 * Copyright 2012, 2017 INFN
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

#include "NodeOperation.h"
#include "../../batch/agent/AgentProcessController.h"

#include <chaos_service_common/data/data.h>

using namespace chaos::metadata_service::api::agent;

#define INFO INFO_LOG(RemoveNodeAssociation)
#define ERR  DBG_LOG(RemoveNodeAssociation)
#define DBG  ERR_LOG(RemoveNodeAssociation)

using namespace chaos::common::data;
using namespace chaos::common::data::structured;
using namespace chaos::service_common::data::agent;
using namespace chaos::metadata_service::api::agent;
using namespace chaos::metadata_service::persistence::data_access;

CHAOS_MDS_DEFINE_API_CLASS_CD(NodeOperation, "nodeOperation");

CDWUniquePtr NodeOperation::execute(CDWUniquePtr api_data) {
    //check for mandatory attributes
    CHECK_CDW_THROW_AND_LOG(api_data, ERR, -1, "No parameter found");
    CHECK_KEY_THROW_AND_LOG(api_data, NodeDefinitionKey::NODE_UNIQUE_ID, ERR, -2, CHAOS_FORMAT("The key %1% is mandatory", %NodeDefinitionKey::NODE_UNIQUE_ID));
    CHAOS_LASSERT_EXCEPTION(api_data->isStringValue(NodeDefinitionKey::NODE_UNIQUE_ID), ERR, -3, CHAOS_FORMAT("The key %1% need to be a string", %NodeDefinitionKey::NODE_UNIQUE_ID));
    CHAOS_LASSERT_EXCEPTION(api_data->isInt32Value("node_operation"), ERR, -3, CHAOS_FORMAT("The key %1% need to be a string", %"node_operation"));
    
    //we can rpocessd
    GET_DATA_ACCESS(AgentDataAccess, a_da, -4);
    int err = 0;
    uint64_t cmd_id = 0;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> batch_data(new CDataWrapper());
    api_data->copyKeyTo(NodeDefinitionKey::NODE_UNIQUE_ID, *batch_data);
    api_data->copyKeyTo("node_operation", *batch_data);
    std::string agent_uid;
    const std::string associated_node_uid = batch_data->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID);
    if((err = a_da->getAgentForNode(associated_node_uid, agent_uid))) {
        LOG_AND_TROW(ERR, -7, CHAOS_FORMAT("Error finding the agent for the node %1% with error %2%", %associated_node_uid%err));
    } else if(agent_uid.size() == 0) {
        LOG_AND_TROW(ERR, -8, CHAOS_FORMAT("The node %1% is not associated to any agent and can't be launched", %associated_node_uid));
    }
    cmd_id = getBatchExecutor()->submitCommand(GET_MDS_COMMAND_ALIAS(batch::agent::AgentProcessController),
                                                   batch_data.release(),
                                                   0,
                                                   1000);
    return CDWUniquePtr();
}
