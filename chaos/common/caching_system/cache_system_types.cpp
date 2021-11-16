#include "cache_system_types.h"
#include <chaos/common/configuration/GlobalConfiguration.h>

namespace chaos {
namespace common {
namespace cache_system {
int CacheDriverSetting::init(chaos::common::data::CDataWrapper& cs) {
  try {
    if (cs.hasKey(OPT_CACHE_DRIVER)) {
      cache_driver_impl = cs.getStringValue(OPT_CACHE_DRIVER);
      LDBG_ << "cache implementation:" << cache_driver_impl;

    } else {
      LERR_ << "missing cache implementation:" << cs.getJSONString();

      return -1;
    }
    if (cs.hasKey(OPT_CACHE_SERVER_LIST) && cs.isVectorValue(OPT_CACHE_SERVER_LIST)) {
      chaos::common::data::CMultiTypeDataArrayWrapperSPtr v = cs.getVectorValue(OPT_CACHE_SERVER_LIST);
      if (v.get() && v->size()) {
        startup_chache_servers = *v;
        LDBG_ << " startup servers:" << startup_chache_servers.size() << " " << startup_chache_servers[0];
      } else {
        LERR_ << "missing startup servers vector:" << cs.getJSONString();

        return -2;
      }
    }
    if (cs.hasKey(OPT_CACHE_LOG_METRIC_UPDATE_INTERVAL)) {
      log_metric_update_interval = cs.getInt32Value(OPT_CACHE_LOG_METRIC_UPDATE_INTERVAL);
    }
    if (cs.hasKey(OPT_CACHE_DRIVER_POOL_MIN_INSTANCE)) {
      caching_pool_min_instances_number = cs.getInt32Value(OPT_CACHE_DRIVER_POOL_MIN_INSTANCE);
    }
    if (cs.hasKey(OPT_CACHE_LOG_METRIC)) {
      log_metric = cs.getBoolValue(OPT_CACHE_LOG_METRIC);
    }
    if (cs.hasKey(OPT_CACHE_DRIVER_KVP) && cs.isVectorValue(OPT_CACHE_DRIVER_KVP)) {
      chaos::common::data::CMultiTypeDataArrayWrapperSPtr v = cs.getVectorValue(OPT_CACHE_DRIVER_KVP);
      if (v.get()) {
        std::vector<std::string> a = *v;
        chaos::GlobalConfiguration::fillKVParameter(key_value_custom_param, a, "[a-zA-Z0-9/_-]+:[a-zA-Z0-9/_-]+");
        for (std::map<std::string, std::string>::iterator i = key_value_custom_param.begin(); i != key_value_custom_param.end(); i++) {
          LDBG_ << i->first << "=" << i->second;
        }
      }
    }
    return 0;

  } catch (...) {
  }
  return -1;
}
}  // namespace cache_system
}  // namespace common
}  // namespace chaos