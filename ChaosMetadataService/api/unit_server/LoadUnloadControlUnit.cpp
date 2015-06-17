/*
 *	LoadUnloadControlUnit.cpp
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

#include "LoadUnloadControlUnit.h"
#include "ChaosMetadataService/batch/unit_server/LoadUnloadControlUnit.h"

#include <boost/format.hpp>

using namespace chaos::common::data;
using namespace chaos::metadata_service::api::unit_server;
using namespace chaos::metadata_service::persistence::data_access;

#define CU_LOUNLO_INFO INFO_LOG(LoadUnloadControlUnit)
#define CU_LOUNLO_DBG  DBG_LOG(LoadUnloadControlUnit)
#define CU_LOUNLO_ERR  ERR_LOG(LoadUnloadControlUnit)

LoadUnloadControlUnit::LoadUnloadControlUnit():
AbstractApi("loadUnloadControlUnit"){

}

LoadUnloadControlUnit::~LoadUnloadControlUnit() {

}

CDataWrapper *LoadUnloadControlUnit::execute(CDataWrapper *api_data,
                                             bool& detach_data) throw(chaos::CException) {

    if(!api_data) {LOG_AND_TROW(CU_LOUNLO_ERR, -1, "Search parameter are needed");}
    if(!api_data->hasKey(chaos::NodeDefinitionKey::NODE_UNIQUE_ID)) {LOG_AND_TROW(CU_LOUNLO_ERR, -1, "The ndk_unique_id key (representing the control unit uid) is mandatory");}
    if(!api_data->hasKey("load")) {LOG_AND_TROW(CU_LOUNLO_ERR, -2, "The 'load' key is mandatory!");}

    int err = 0;
    uint64_t command_id = 0;
    CDataWrapper *us_base_description = NULL;
    CDataWrapper *cu_base_descirption = NULL;
    CDataWrapper *cu_instance_description = NULL;
    std::auto_ptr<CDataWrapper> load_unload_data_pack(new CDataWrapper());

        //get the parameter
    const std::string cu_uid = api_data->getStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID);
    const bool load_unload = api_data->getBoolValue("load");

    load_unload_data_pack->addStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID, cu_uid);
    load_unload_data_pack->addBoolValue("load", load_unload);

        //get data access
    GET_DATA_ACCESS(NodeDataAccess, n_da, -3)
    GET_DATA_ACCESS(ControlUnitDataAccess, cu_da, -4)

    CU_LOUNLO_DBG << "Starting the " << (load_unload?"load":"unload") << " phase for " << cu_uid;
    if((err = n_da->getNodeDescription(cu_uid, &cu_base_descirption))){
        LOG_AND_TROW(CU_LOUNLO_ERR, err, boost::str(boost::format("Error fetching the base information for cuid:%1%") % cu_uid));
    } else if(!cu_base_descirption) {
        LOG_AND_TROW(CU_LOUNLO_ERR, -5, boost::str(boost::format("No base infromation found for control unit:%1%") % cu_uid));
    } else {
        std::auto_ptr<CDataWrapper> cu_inf(cu_base_descirption);
        if(cu_inf->hasKey(chaos::NodeDefinitionKey::NODE_TYPE)) {
        std:string type = cu_inf->getStringValue(chaos::NodeDefinitionKey::NODE_TYPE);
            if(type.compare(chaos::NodeType::NODE_TYPE_CONTROL_UNIT) != 0) {
                LOG_AND_TROW(CU_LOUNLO_ERR, -6, boost::str(boost::format("The node id '%1%' is not a control unit") % cu_uid));
            }
        } else {
            LOG_AND_TROW(CU_LOUNLO_ERR, -7, "The node doesn't has a type attribute");
        }
            //we have the information, ad need to add the load attribute to this.
        cu_inf->addBoolValue("load", load_unload);
            //now we need to fetch the instance configuratio
        if((err = cu_da->getInstanceDescription(cu_uid,
                                                &cu_instance_description)) ||
           (cu_instance_description == NULL)) {
                //we haven't found an instance for the node
            LOG_AND_TROW(CU_LOUNLO_ERR, -8, "The node doesn't has an instance configured");
        } else {
            std::auto_ptr<CDataWrapper> cu_instance(cu_instance_description);

            if(!cu_instance->hasKey(chaos::NodeDefinitionKey::NODE_PARENT)) {
                LOG_AND_TROW(CU_LOUNLO_ERR, -9, "Control unit isntace lake of parent key(unit server)");
            }

            if(!cu_instance->hasKey("control_unit_implementation")) {
                LOG_AND_TROW(CU_LOUNLO_ERR, -10, "No implementation found into control unit instances");
            }

            if((err = n_da->getNodeDescription(cu_instance->getStringValue(chaos::NodeDefinitionKey::NODE_PARENT), &us_base_description)) ||
               us_base_description == NULL){
                LOG_AND_TROW(CU_LOUNLO_ERR, err, "Error fetching unit server information");
            }

            std::auto_ptr<CDataWrapper> us_instance(us_base_description);
            if(!us_instance->hasKey(chaos::NodeDefinitionKey::NODE_RPC_ADDR)) {
                LOG_AND_TROW(CU_LOUNLO_ERR, -11, "No rpc address for unit server found");
            }

                //set the type of the control unit to instance and the rpc addres sof the unit server
            load_unload_data_pack->addStringValue(chaos::NodeDefinitionKey::NODE_RPC_ADDR, us_instance->getStringValue(chaos::NodeDefinitionKey::NODE_RPC_ADDR));
            if(load_unload) {
                    //in load phase we need the type to instantiate the control unit
                load_unload_data_pack->addStringValue(UnitServerNodeDomainAndActionRPC::PARAM_CONTROL_UNIT_TYPE, cu_instance->getStringValue("control_unit_implementation"));
            }
                //perform phase in background
            getBatchExecutor()->submitCommand(std::string(GET_MDS_COMMAND_ALIAS(batch::unit_server::LoadUnloadControlUnit)),
                                              load_unload_data_pack.release(),
                                              command_id);
        }
    }
    return NULL;
}