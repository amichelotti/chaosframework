/*
 *	AgentLoggingApiGroup.cpp
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 07/03/2017 INFN, National Institute of Nuclear Physics
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

#include "AgentLoggingApiGroup.h"
#include "ManageNodeLogging.h"
#include "GetProcessLogEntries.h"
using namespace chaos::metadata_service::api::agent::logging;

DEFINE_CLASS_FACTORY_NO_ALIAS(AgentLoggingApiGroup,
                              chaos::metadata_service::api::AbstractApiGroup);

AgentLoggingApiGroup::AgentLoggingApiGroup():
AbstractApiGroup(AgentNodeDomainAndActionRPC::LogWorker::RPC_DOMAIN){
    addApi<ManageNodeLogging>();
    addApi<GetProcessLogEntries>();
}

AgentLoggingApiGroup::~AgentLoggingApiGroup() {}