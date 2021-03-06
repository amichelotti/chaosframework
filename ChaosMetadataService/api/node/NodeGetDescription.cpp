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

#include "NodeGetDescription.h"

#define USRA_INFO INFO_LOG(NodeRegister)
#define USRA_DBG  DBG_LOG(NodeRegister)
#define USRA_ERR  ERR_LOG(NodeRegister)

using namespace chaos::common::data;
using namespace chaos::metadata_service::api::node;
using namespace chaos::metadata_service::persistence::data_access;

CHAOS_MDS_DEFINE_API_CLASS_CD(NodeGetDescription, "getNodeDescription")

CDWUniquePtr NodeGetDescription::execute(CDWUniquePtr api_data) {
    int err = 0;
    bool presence = false;
    chaos::common::data::CDataWrapper *result = NULL;
    if(!api_data->hasKey(NodeDefinitionKey::NODE_UNIQUE_ID)) {
        LOG_AND_TROW(USRA_ERR, -1, "Node unique id not set as parameter")
    }
    
    //!get the unit server data access
    GET_DATA_ACCESS(NodeDataAccess, n_da, -2)
    if((err = n_da->checkNodePresence(presence,
                                      api_data->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID)))) {
        LOG_AND_TROW(USRA_ERR, err, "Error checking node presence")
    } else if(presence){
        if((err = n_da->getNodeDescription(api_data->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID), &result))) {
            LOG_AND_TROW(USRA_ERR, err, "Error fetching node  '"+   api_data->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID)+"' decription")
        }
    } else {
        LOG_AND_TROW(USRA_ERR, -3, "Node '"+   api_data->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID)+"' not found")
    }
    return CDWUniquePtr(result);
}
