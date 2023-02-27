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

#include <chaos_metadata_service_client/api_proxy/logging/DeleteLog.h>
#include <chaos/common/message/MultiAddressMessageRequestFuture.h>
using namespace chaos;
using namespace chaos::common::data;
using namespace chaos::metadata_service_client::api_proxy;
using namespace chaos::metadata_service_client::api_proxy::logging;

API_PROXY_CD_DEFINITION(DeleteLog,
                        MetadataServerLoggingDefinitionKeyRPC::ACTION_NODE_LOGGING_RPC_DOMAIN,
                        "deleteLog")

ApiProxyResult DeleteLog::execute(const std::string& search_string,
                                const std::string& domain_list,
                                       const uint64_t start_ts) {
    CDWUniquePtr pack(new CDataWrapper());
    pack->addStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_SOURCE_IDENTIFIER, search_string);
    pack->addStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_DOMAIN, domain_list);
    pack->addInt64Value(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP, start_ts);

   
    return callApi(pack);
}

