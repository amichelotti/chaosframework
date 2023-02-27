/*
 * Copyright 2023 INFN
 *
 * Andrea Michelotti
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

#include "DeleteLog.h"

#include <chaos/common/network/NetworkBroker.h>

using namespace chaos::metadata_service::api::logging;

#define L_SE_INFO INFO_LOG(DeleteLog)
#define L_SE_DBG  DBG_LOG(DeleteLog)
#define L_SE_ERR  ERR_LOG(DeleteLog)

using namespace chaos::common::data;
using namespace chaos::common::network;
using namespace chaos::common::event::channel;
using namespace chaos::metadata_service::api::logging;
using namespace chaos::metadata_service::persistence::data_access;

CHAOS_MDS_DEFINE_API_CLASS_CD(DeleteLog, "deleteLog")



CDWUniquePtr DeleteLog::execute(CDWUniquePtr api_data) {
    int err = 0;
    GET_DATA_ACCESS(LoggingDataAccess, l_da, -4);

    //check for mandatory attributes
    CHECK_CDW_THROW_AND_LOG(api_data, L_SE_ERR, -1, "No parameter found");
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_SOURCE_IDENTIFIER, L_SE_ERR, -2, "The log timestamp key is mandatory:"+api_data->getJSONString());
    CHAOS_LASSERT_EXCEPTION(api_data->isStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_SOURCE_IDENTIFIER), L_SE_ERR, -3, "The log timestamp key needs to be a string value");
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP, L_SE_ERR, -4, "The log timestamp key is mandatory:"+api_data->getJSONString());
    CHAOS_LASSERT_EXCEPTION(api_data->isInt64Value(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP), L_SE_ERR, -5, "The log timestamp key needs to be an int64 value");
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_DOMAIN, L_SE_ERR, -6, "The log domain key is mandatory:"+api_data->getJSONString());
    CHAOS_LASSERT_EXCEPTION(api_data->isStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_DOMAIN), L_SE_ERR, -7, "The log domain needs to be a string");
   
    
    //crete entry
    LogEntry new_log_entry;
    new_log_entry.source_identifier = api_data->getStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_SOURCE_IDENTIFIER);
    new_log_entry.ts = api_data->getUInt64Value(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP);
    new_log_entry.domain = api_data->getStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_DOMAIN);
    //compelte log antry with log channel custom key
   
    //insert the log entry
    if((err = l_da->eraseLogBeforTS(new_log_entry.source_identifier,new_log_entry.domain,new_log_entry.ts))){
        LOG_AND_TROW(L_SE_ERR, -9, "Error erasing log :"+api_data->getJSONString());
    }
    
   
    return NULL;
}

