/*
 *	GetSetFullUnitServer.cpp
 *	!CHAOS
 *	Created by Andrea Michelotti.
 *
 *    	Copyright 2017 INFN, National Institute of Nuclear Physics
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

#include <ChaosMetadataServiceClient/api_proxy/unit_server/GetSetFullUnitServer.h>

using namespace chaos;
using namespace chaos::metadata_service_client::api_proxy;
using namespace chaos::metadata_service_client::api_proxy::unit_server;

API_PROXY_CD_DEFINITION(GetSetFullUnitServer,
                        "unit_server",
                        "GetSetFullUnitServer");

/*!
 
 */
ApiProxyResult GetSetFullUnitServer::execute(const std::string& unit_server_uid, const chaos::common::data::CDataWrapper& usdesc, int reset) {
    chaos::common::data::CDataWrapper *message = new chaos::common::data::CDataWrapper();
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, unit_server_uid);
    message->addCSDataValue("fulldesc",usdesc);
    message->addInt32Value("operation", reset);
    return callApi(message);
}

ApiProxyResult GetSetFullUnitServer::execute(const std::string& unit_server_uid) {
    chaos::common::data::CDataWrapper *message = new chaos::common::data::CDataWrapper();
    message->addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, unit_server_uid);
    return callApi(message);
}