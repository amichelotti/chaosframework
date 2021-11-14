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

#include "RetrieveMultipleData.h"

#include <chaos/common/chaos_types.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos_service_common/DriverPoolManager.h>
#include "../../object_storage/abstraction/ObjectStorageDataAccess.h"
#define INFO INFO_LOG(RetrieveMultipleData)
#define DBG  DBG_LOG(RetrieveMultipleData)
#define ERR  ERR_LOG(RetrieveMultipleData)

using namespace chaos::common::data;
using namespace chaos::metadata_service::api::service;
using namespace chaos::metadata_service::persistence::data_access;
using namespace chaos::metadata_service::object_storage::abstraction;
using namespace chaos::service_common;
using namespace chaos::common::cache_system;

CHAOS_MDS_DEFINE_API_CLASS_CD(RetrieveMultipleData, "retrieveMultipleData")

CDWUniquePtr RetrieveMultipleData::execute(CDWUniquePtr api_data) {
    int err=-1;
    CHECK_CDW_THROW_AND_LOG(api_data, ERR, -1, "No parameter found");
    CDWUniquePtr result(new CDataWrapper());

    ChaosStringVector nodes;
    chaos::common::data::VectorCDWShrdPtr res;
    if(api_data->hasKey("nodes")&&api_data->isVectorValue("nodes")){
         CMultiTypeDataArrayWrapperSPtr d = api_data->getVectorValue("tags");
        for(int idx = 0;idx < d->size();idx++) {
            nodes.push_back(d->getStringElementAtIndex(idx));
        }
        err=execute(nodes,res);
        if(err==0){
            for(VectorObject::iterator i=res.begin();i!=res.end();i++){
             result->appendCDataWrapperToArray(*(i->get()));   
            }
            result->finalizeArrayForKey("data");

        }
    } else {
        ERR<<" NO 'nodes' list provided";
                
    }
   
    result->addInt32Value("error",err);

    return result;
    
}
int RetrieveMultipleData::execute(const ChaosStringVector& keys,chaos::common::data::VectorCDWShrdPtr& result){
    int err=0;
CacheDriver& cache_slot = DriverPoolManager::getInstance()->getCacheDrv();
  try {
    //get data
    MultiCacheData multi_cached_data;
    err = cache_slot.getData(keys,
                             multi_cached_data);
    for (ChaosStringVectorConstIterator it  = keys.begin(),
                                        end = keys.end();
         it != end;
         it++) {
      const CacheData& cached_element = multi_cached_data[*it];
      if (!cached_element ||
          cached_element->size() == 0) {
            result.push_back(chaos::common::data::CDWShrdPtr());
       
      } else {
        result.push_back(chaos::common::data::CDWShrdPtr((chaos::common::data::CDataWrapper*)cached_element->data()));

       
      }
    }

  } catch (...) {
      ERR<<" CATCH ERROR";
      err=-666;
  }
  return err;
    
   
}
