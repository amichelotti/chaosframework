/*
 *	ServiceApiGroup.cpp
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

#include "ResetAll.h"
#include "GetAllSnapshot.h"
#include "DeleteSnapshot.h"
#include "RestoreSnapshot.h"
#include "ServiceApiGroup.h"
#include "CreateNewSnapshot.h"
#include "GetSnapshotForNode.h"
#include "GetNodesForSnapshot.h"
#include "GetSnapshotDatasetsForNode.h"

#include "SetVariable.h"
#include "GetVariable.h"
#include "RemoveVariable.h"

using namespace chaos::metadata_service::api::service;

DEFINE_CLASS_FACTORY_NO_ALIAS(ServiceApiGroup,
                              chaos::metadata_service::api::AbstractApiGroup);

ServiceApiGroup::ServiceApiGroup():
AbstractApiGroup("service"){
    addApi<ResetAll>();
    addApi<GetAllSnapshot>();
    addApi<DeleteSnapshot>();
    addApi<RestoreSnapshot>();
    addApi<CreateNewSnapshot>();
    addApi<GetSnapshotForNode>();
    addApi<GetNodesForSnapshot>();
    addApi<GetSnapshotDatasetsForNode>();
    
    addApi<SetVariable>();
    addApi<GetVariable>();
    addApi<RemoveVariable>();
}

ServiceApiGroup::~ServiceApiGroup() {}
