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
#include <chaos/common/global.h>
#include <chaos/common/utility/TimingUtil.h>
using namespace chaos::common::cache_system;

CacheDriver::CacheDriver(std::string alias)
    : NamedService(alias) {
  enable_cache_for_ms.clear();
  first_level_cache.clear();
}

CacheDriver::~CacheDriver() {}

std::map<std::string, std::pair<int64_t, chaos::common::data::CDWShrdPtr> > CacheDriver::first_level_cache;
std::map<std::string, int32_t>                                              CacheDriver::enable_cache_for_ms;
//! init
/*!
 Need a point to a structure DBDriverSetting for the setting
 */
void CacheDriver::init(void* init_data) {
  if (init_data != NULL) {
    cache_settings = *static_cast<CacheDriverSetting*>(init_data);
  }
  enable_cache_for_ms.clear();
  first_level_cache.clear();
}

//! deinit
void CacheDriver::deinit() {
  enable_cache_for_ms.clear();
  first_level_cache.clear();
}
chaos::common::data::CDWShrdPtr CacheDriver::getData(const std::string& key) {
  if ((enable_cache_for_ms.find(key)!=enable_cache_for_ms.end()) && (first_level_cache.find(key)!=first_level_cache.end())) {
    uint64_t now = chaos::common::utility::TimingUtil::getTimeStamp();
    if ((now - first_level_cache[key].first) < enable_cache_for_ms[key]) {
      LDBG_ << "retrive from caching:" << key;

      return first_level_cache[key].second;
    }
  }
  CacheData                       d;
  chaos::common::data::CDWShrdPtr ret;
  if (getData(key, d) == 0) {
    if (d.get() && d->size()) {
      chaos::common::data::CDataWrapper* tmp = new chaos::common::data::CDataWrapper(d->data(), d->size());
      ret.reset(tmp);
      if (enable_cache_for_ms.find(key)!=enable_cache_for_ms.end()) {
        uint64_t now = chaos::common::utility::TimingUtil::getTimeStamp();
        LDBG_ << "mupdate caching:" << key;
        first_level_cache[key] = {now, ret};
      }
    }
  }
  return ret;
}
void CacheDriver::enableCache(const std::string& key, int32_t validity_ms) {
  if (validity_ms > 0) {
    enable_cache_for_ms[key] = validity_ms;
    LDBG_ << "enabling caching for key:" << key << " validity:" << validity_ms << " ms";

  } else {
    LDBG_ << "disabling caching for key:" << key;
    enable_cache_for_ms.erase(key);
  }
}

std::vector<chaos::common::data::CDWShrdPtr> CacheDriver::getData(const ChaosStringVector& keys) {
  std::vector<chaos::common::data::CDWShrdPtr> ret;
  MultiCacheData                               multi_cached_data;
  std::map<std::string, bool>                  is_cached;
  uint64_t                                     now = chaos::common::utility::TimingUtil::getTimeStamp();

  int res = 0;
  if (enable_cache_for_ms.size() == 0) {
    res = getData(keys, multi_cached_data);
  } else {
    ChaosStringVector nocached;
    for (ChaosStringVector::const_iterator i = keys.begin(); i != keys.end(); i++) {
      if ((enable_cache_for_ms.find(*i)!=enable_cache_for_ms.end()) && (first_level_cache.find(*i)!=first_level_cache.end())) {
        if ((now - first_level_cache[*i].first) < enable_cache_for_ms[*i]) {
          is_cached[*i] = true;
        } else {
          nocached.push_back(*i);
          is_cached[*i] = false;
        }
      } else {
        nocached.push_back(*i);
        is_cached[*i] = false;
      }
    }
    res = getData(nocached, multi_cached_data);
  }
  if (res == 0) {
    for (ChaosStringVectorConstIterator it  = keys.begin(),
                                        end = keys.end();
         it != end;
         it++) {
      if (((is_cached.size() == 0) || (is_cached[*it] == false))) {
        const CacheData& cached_element = multi_cached_data[*it];
        if ((cached_element.get() == NULL) || (cached_element->size() == 0)) {
          ret.push_back(chaos::common::data::CDWShrdPtr());
        } else {
          chaos::common::data::CDWShrdPtr r = chaos::common::data::CDWShrdPtr(new chaos::common::data::CDataWrapper(cached_element->data(), cached_element->size()));
          ret.push_back(r);
          if (enable_cache_for_ms.find(*it)!=enable_cache_for_ms.end()) {
            if (enable_cache_for_ms[*it] > 0) {
              LDBG_ << "mupdate caching:" << *it;
              first_level_cache[*it] = {now, r};
            }
          }
        }
      } else {
        //    LDBG_ << "mretrive from caching:" << *it;

        ret.push_back(first_level_cache[*it].second);
      }
    }
  } else {
    LERR_ << __PRETTY_FUNCTION__ << " Error getting data from cache, ret:" << res;
  }
  return ret;
}