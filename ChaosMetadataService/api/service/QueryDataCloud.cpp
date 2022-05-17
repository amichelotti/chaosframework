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

#include "QueryDataCloud.h"

#include <chaos/common/chaos_types.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos_service_common/DriverPoolManager.h>
#include "../../object_storage/abstraction/ObjectStorageDataAccess.h"
#define INFO INFO_LOG(QueryDataCloud)
#define DBG  DBG_LOG(QueryDataCloud)
#define ERR  ERR_LOG(QueryDataCloud)

using namespace chaos::common::data;
using namespace chaos::metadata_service::api::service;
using namespace chaos::metadata_service::persistence::data_access;
using namespace chaos::metadata_service::object_storage::abstraction;
using namespace chaos::service_common;

CHAOS_MDS_DEFINE_API_CLASS_CD(QueryDataCloud, "queryDataCloud")

CDWUniquePtr QueryDataCloud::execute(CDWUniquePtr api_data) {
    CHECK_CDW_THROW_AND_LOG(api_data, ERR, -1, "No parameter found");
    CHECK_KEY_THROW_AND_LOG(api_data, NodeDefinitionKey::NODE_UNIQUE_ID, ERR, -2, "The node unique id key is mandatory");
    CHECK_ASSERTION_THROW_AND_LOG(api_data->isStringValue(NodeDefinitionKey::NODE_UNIQUE_ID), ERR, -3, "The node unique id needs to be a string");
    
    int err = 0;
    const std::string node_uid = api_data->getStringValue(NodeDefinitionKey::NODE_UNIQUE_ID);
    int64_t start_ts=0,end_ts=0,runid=0,start_seq=0;
    uint32_t page=10000;
    if(api_data->hasKey("start_ts")){
        start_ts=api_data->getInt64Value("start_ts");
    }
    if(api_data->hasKey("end_ts")){
        end_ts=api_data->getInt64Value("end_ts");
    }
    if(api_data->hasKey("runid")){
        runid=api_data->getInt64Value("runid");
    }
    if(api_data->hasKey("seq")){
        start_seq=api_data->getInt64Value("seq");
    }
    if(api_data->hasKey("page")){
        page=api_data->getInt32Value("page");
    }
    ChaosStringSet meta_tags;
     ChaosStringSet projection_keys;
    if(api_data->hasKey("tags")&&api_data->isVectorValue("tags")){
         CMultiTypeDataArrayWrapperSPtr d = api_data->getVectorValue("tags");
        for(int idx = 0;idx < d->size();idx++) {
            meta_tags.insert(d->getStringElementAtIndex(idx));
        }
    }
    if(api_data->hasKey("prj")&&api_data->isVectorValue("prj")){
         CMultiTypeDataArrayWrapperSPtr d = api_data->getVectorValue("prj");
        for(int idx = 0;idx < d->size();idx++) {
            projection_keys.insert(d->getStringElementAtIndex(idx));
        }

    }
    VectorObject found_object_page;
    ChaosUniquePtr<chaos::common::data::CDataWrapper> result(new CDataWrapper());

    chaos::common::direct_io::channel::opcode_headers::SearchSequence last_record_found_seq;
    last_record_found_seq.run_id=runid;
    last_record_found_seq.datapack_counter=start_seq;

    DBG<<node_uid<<" start:"<<start_ts<<"("<<chaos::common::utility::TimingUtil::toString(start_ts)<<" end:"<<end_ts<<"("<<chaos::common::utility::TimingUtil::toString(end_ts)<<" page:"<<page<<" runid:"<<last_record_found_seq.run_id<<" seq:"<<last_record_found_seq.datapack_counter<<"tag:"<<((meta_tags.size())?*meta_tags.begin():"");

    int res=execute(node_uid,meta_tags,projection_keys,start_ts,end_ts,page,last_record_found_seq,found_object_page);
    if(res==0){
        if(last_record_found_seq.ts>end_ts){
            ERR<<node_uid<<" BAD last timestamp RETURNED:"<<found_object_page.size()<<" ELEMENTS, runid:"<<last_record_found_seq.run_id<<" seq:"<<last_record_found_seq.datapack_counter<< "ts:"<<last_record_found_seq.ts<<"("<<chaos::common::utility::TimingUtil::toString(last_record_found_seq.ts)<<") > "<<end_ts<<"("<<chaos::common::utility::TimingUtil::toString(end_ts)<<")";

        } else {
            DBG<<node_uid<<" RETURNED:"<<found_object_page.size()<<" ELEMENTS, runid:"<<last_record_found_seq.run_id<<" seq:"<<last_record_found_seq.datapack_counter<< "ts:"<<last_record_found_seq.ts<<"("<<chaos::common::utility::TimingUtil::toString(last_record_found_seq.ts)<<")";
        }

        for(VectorObject::iterator i=found_object_page.begin();i!=found_object_page.end();i++){
             result->appendCDataWrapperToArray(*(i->get()));   
        }

        
    }
    result->finalizeArrayForKey("data");

    result->addInt64Value("runid",last_record_found_seq.run_id);
    result->addInt64Value("seq",last_record_found_seq.datapack_counter);
    result->addInt64Value("ts",last_record_found_seq.ts);
    result->addInt32Value("error",res);

    return result;
    
}

int QueryDataCloud::execute(const std::string& node_uid,
                                       const ChaosStringSet& meta_tags,
                                       const ChaosStringSet& projection_keys,
                                       const uint64_t start_ts,
                                       const uint64_t end_ts,
                                       const uint32_t page,
                                       chaos::common::direct_io::channel::opcode_headers::SearchSequence& last_sequence,
                                        chaos::common::direct_io::channel::opcode_headers::QueryResultPage& found_element_page
                                        ){

    ObjectStorageDataAccess *obj_storage_da  = DriverPoolManager::getInstance()->getObjectStorageDrv().getDataAccess<ObjectStorageDataAccess>();
    CHAOS_ASSERT(obj_storage_da);
    return obj_storage_da->findObject(node_uid,
                                           meta_tags,
                                           projection_keys,
                                           start_ts,
                                           end_ts,
                                           page,
                                           found_element_page,
                                           last_sequence);
   
}
