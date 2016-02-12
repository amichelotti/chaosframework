/*
 *	SubmitEntry.cpp
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by Claudio Bisegni.
 *
 *    	Copyright 11/02/16 INFN, National Institute of Nuclear Physics
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

#include "SubmitEntry.h"

#include <chaos/common/network/NetworkBroker.h>

using namespace chaos::metadata_service::api::logging;

#define L_SE_INFO INFO_LOG(SubmitEntry)
#define L_SE_DBG  DBG_LOG(SubmitEntry)
#define L_SE_ERR  ERR_LOG(SubmitEntry)

using namespace chaos::common::data;
using namespace chaos::common::network;
using namespace chaos::common::event::channel;
using namespace chaos::metadata_service::api::logging;
using namespace chaos::metadata_service::persistence::data_access;

SubmitEntry::SubmitEntry():
AbstractApi(MetadataServerLoggingDefinitionKeyRPC::ACTION_NODE_LOGGING_SUBMIT_ENTRY){
    alert_event_channel = NetworkBroker::getInstance()->getNewAlertEventChannel();
}

SubmitEntry::~SubmitEntry() {
    if(alert_event_channel) {NetworkBroker::getInstance()->disposeEventChannel(alert_event_channel);}
}

chaos::common::data::CDataWrapper *SubmitEntry::execute(CDataWrapper *api_data, bool& detach_data) {
    int err = 0;
    
    //check for mandatory attributes
    CHECK_CDW_THROW_AND_LOG(api_data, L_SE_ERR, -1, "No parameter found");
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_SOURCE_IDENTIFIER, L_SE_ERR, -2, "The log timestamp key is mandatory");
    CHAOS_LASSERT_EXCEPTION(api_data->isStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_SOURCE_IDENTIFIER), L_SE_ERR, -3, "The log timestamp key needs to be a string value");
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP, L_SE_ERR, -4, "The log timestamp key is mandatory");
    CHAOS_LASSERT_EXCEPTION(api_data->isInt64Value(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP), L_SE_ERR, -5, "The log timestamp key needs to be an int64 value");
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_DOMAIN, L_SE_ERR, -6, "The log domain key is mandatory");
    CHAOS_LASSERT_EXCEPTION(api_data->isStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_DOMAIN), L_SE_ERR, -7, "The log domain needs to be a string");
    
    GET_DATA_ACCESS(LoggingDataAccess, l_da, -8);
    
    //crete entry
    LogEntry new_log_entry;
    new_log_entry.source_identifier = api_data->getStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_SOURCE_IDENTIFIER);
    new_log_entry.ts = api_data->getUInt64Value(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_TIMESTAMP);
    new_log_entry.domain = api_data->getStringValue(MetadataServerLoggingDefinitionKeyRPC::PARAM_NODE_LOGGING_LOG_DOMAIN);
    if(new_log_entry.domain.compare("error") == 0) {
        completeErrorLogEntry(api_data,
                              new_log_entry);
    }
    if((err = l_da->insertNewEntry(new_log_entry))){
        LOG_AND_TROW(L_SE_ERR, -9, "Error creaating new log entry");
    }
    return NULL;
}

void SubmitEntry::completeErrorLogEntry(CDataWrapper *api_data,
                                       LogEntry& new_log_entry) {
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_CODE, L_SE_ERR, -1, "The log error code key is mandatory");
    CHAOS_LASSERT_EXCEPTION(api_data->isInt32Value(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_CODE), L_SE_ERR, -2, "The error code  key needs to be an int32 value");
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_MESSAGE, L_SE_ERR, -3, "The error message key is mandatory");
    CHAOS_LASSERT_EXCEPTION(api_data->isStringValue(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_MESSAGE), L_SE_ERR, -4, "The error message key needs to be a string value");
    CHECK_KEY_THROW_AND_LOG(api_data, MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_DOMAIN, L_SE_ERR, -5, "The log domain key is mandatory");
    CHAOS_LASSERT_EXCEPTION(api_data->isStringValue(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_DOMAIN), L_SE_ERR, -6, "The log domain needs to be a string");
    
    new_log_entry.map_string_value.insert(make_pair(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_MESSAGE,
                                                    api_data->getStringValue(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_MESSAGE)));
    new_log_entry.map_string_value.insert(make_pair(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_DOMAIN,
                                                    api_data->getStringValue(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_DOMAIN)));
    new_log_entry.map_int32_value.insert(make_pair(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_CODE,
                                                    api_data->getInt32Value(MetadataServerLoggingDefinitionKeyRPC::ErrorLogging::PARAM_NODE_LOGGING_LOG_ERROR_CODE)));
}