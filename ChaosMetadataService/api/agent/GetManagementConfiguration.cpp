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

#include "GetManagementConfiguration.h"
#include "../../batch/agent/AgentCheckAgentProcess.h"

#include <chaos_service_common/data/data.h>

using namespace chaos::metadata_service::api::agent;

#define INFO INFO_LOG(GetManagementConfiguration)
#define ERR  DBG_LOG(GetManagementConfiguration)
#define DBG  ERR_LOG(GetManagementConfiguration)

using namespace chaos::common::data;
using namespace chaos::common::data::structured;
using namespace chaos::service_common::data::agent;
using namespace chaos::metadata_service::api::agent;
using namespace chaos::metadata_service::persistence::data_access;

CHAOS_MDS_DEFINE_API_CLASS_CD(GetManagementConfiguration, "getManagementConfiguration")

CDWUniquePtr GetManagementConfiguration::execute(CDWUniquePtr api_data) {
    GET_DATA_ACCESS(AgentDataAccess, a_da, -1);
    AgentManagementSettingSDWrapper set_w;
    a_da->getLogEntryExpiration(set_w().expiration_enabled, set_w().log_expiration_in_seconds);
    return set_w.serialize();
}
