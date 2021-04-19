/*
 * Copyright 2012, 2021 INFN
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
#include "CacheDriver.h"
#include <chaos/common/data/CDataWrapper.h>
using namespace chaos::common::cache_system;
CacheDriverSetting CacheDriver::cache_settings;

CacheDriver::CacheDriver(std::string alias):
NamedService(alias){}

CacheDriver::~CacheDriver() {}

//! init
/*!
 Need a point to a structure DBDriverSetting for the setting
 */
void CacheDriver::init(void *init_data)  {
	if(init_data!=NULL){
			cache_settings = *static_cast<CacheDriverSetting*>(init_data);
	}
}

//! deinit
void CacheDriver::deinit()  {}
chaos::common::data::CDWShrdPtr CacheDriver::getData(const std::string& key){
	CacheData d;
	chaos::common::data::CDWShrdPtr ret;
    if(getData(key,d)==0){
      if(d.get()&&d->size()){
        chaos::common::data::CDataWrapper* tmp = new chaos::common::data::CDataWrapper(d->data(),d->size());
        ret.reset(tmp);
        return ret;
      }
    }
	return ret;
}
std::vector<chaos::common::data::CDWShrdPtr> CacheDriver::getData(const ChaosStringVector&    keys){
	std::vector<chaos::common::data::CDWShrdPtr> ret;
	 MultiCacheData            multi_cached_data;
	 if(getData(keys,multi_cached_data)==0){
		  for(ChaosStringVectorConstIterator it = keys.begin(),
            end = keys.end();
            it != end;
            it++) {
            const CacheData& cached_element = multi_cached_data[*it];
			if((cached_element.get()==NULL) || (cached_element->size() == 0)){
                ret.push_back(chaos::common::data::CDWShrdPtr());
            } else {
                chaos::common::data::CDWShrdPtr r =chaos::common::data::CDWShrdPtr(new chaos::common::data::CDataWrapper(cached_element->data(),cached_element->size()));
                ret.push_back(r);


            }
        }
     
    }
	return ret;

}