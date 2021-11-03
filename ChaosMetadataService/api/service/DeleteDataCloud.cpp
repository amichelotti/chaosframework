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

#include "DeleteDataCloud.h"

#include <chaos/common/chaos_types.h>
#include "../../object_storage/abstraction/ObjectStorageDataAccess.h"
#include <chaos_service_common/DriverPoolManager.h>

#define INFO INFO_LOG(DeleteDataCloud)
#define DBG  DBG_LOG(DeleteDataCloud)
#define ERR  ERR_LOG(DeleteDataCloud)

using namespace chaos::common::data;
using namespace chaos::metadata_service::api::service;
using namespace chaos::metadata_service::persistence::data_access;
using namespace chaos::metadata_service::object_storage::abstraction;
using namespace chaos::service_common;

CHAOS_MDS_DEFINE_API_CLASS_CD(DeleteDataCloud, "deleteDataCloud")
chaos::common::data::CDWUniquePtr DeleteDataCloud::execute(const std::string&uid,uint64_t start,uint64_t end){
    ChaosUniquePtr<chaos::common::data::CDataWrapper> result(new CDataWrapper());
    ObjectStorageDataAccess *obj_storage_da  = DriverPoolManager::getInstance()->getObjectStorageDrv().getDataAccess<ObjectStorageDataAccess>();
    CHAOS_ASSERT(obj_storage_da);
    int err;
    if((err = obj_storage_da->deleteObject(uid,
                                           start,
                                           end
                                          ))){
            ERR << uid<<" Error deletingObject from "<<start<<" to "<<end;


    } 
    result->addInt32Value("error",err);
    return result;
}

CDWUniquePtr DeleteDataCloud::execute(CDWUniquePtr api_data) {
    CHECK_CDW_THROW_AND_LOG(api_data, ERR, -1, "No parameter found");
    CHECK_KEY_THROW_AND_LOG(api_data, NodeDefinitionKey::NODE_UNIQUE_ID, ERR, -2, "The node unique id key is mandatory");
    CHECK_ASSERTION_THROW_AND_LOG(api_data->isStringValue(NodeDefinitionKey::NODE_UNIQUE_ID), ERR, -3, "The node unique id needs to be a string");
    
    int err = 0;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> result(new CDataWrapper());
    const std::string node_uid = api_data->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID);
    
    int64_t start_ts=0,end_ts=0;
    uint32_t page=10000;
    if(api_data->hasKey("start_ts")){
        start_ts=api_data->getInt64Value("start_ts");
    }
    if(api_data->hasKey("end_ts")){
        end_ts=api_data->getInt64Value("end_ts");
    }
    return execute(node_uid,start_ts,end_ts);
    
}
