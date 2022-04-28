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
#include "HealtManagerDirect.h"
#include <chaos_service_common/DriverPoolManager.h>
#include <chaos/common/io/SharedManagedDirecIoDataDriver.h>

//#include <chaos_service_common/ChaosManager.h>
#define HM_INFO INFO_LOG(HealtManagerDirect)
#define HM_DBG DBG_LOG(HealtManagerDirect)
#define HM_ERR ERR_LOG(HealtManagerDirect)

#define HEALT_NEED_NODE_NO_METRIC_PRESENCE(n,m)\
if(map_node.count(n) == 0) return;\
if(map_node[n]->map_metric.count(m) != 0) return;

#define HEALT_NEED_NODE_AND_METRIC_PRESENCE(n,m)\
if(map_node.count(n) == 0) return;\
if(map_node[n]->map_metric.count(m) == 0) return;

#define HEALT_SET_METRIC_VALUE(node_metrics_ptr, t, m, v)\
t *tmp = static_cast<t*>(node_metrics_ptr->map_metric[m].get());\
if(tmp)tmp->value = v;


#define HEALT_GET_METRIC_VALUE(node_metrics_ptr, t, m)\
t *tmp = static_cast<t*>(node_metrics_ptr->map_metric[m].get());\
if(tmp)return tmp->value;

#define HEALT_SET_METRIC_TIMESTAMP_LAST_METRIC(node_metrics_ptr)\
Int64HealtMetric *ts_tmp = static_cast<Int64HealtMetric*>(node_metrics_ptr->map_metric[NodeHealtDefinitionKey::NODE_HEALT_TIMESTAMP_LAST_METRIC].get());\
ts_tmp->value = TimingUtil::getTimeStamp();

using namespace chaos::common::data;
using namespace chaos::common::io;

using namespace chaos::common::healt_system;
namespace chaos {
    namespace service_common{
        namespace health_system {
            

void HealtManagerDirect::_publish(const ChaosSharedPtr<NodeHealtSet>& heath_set,
                            uint64_t publish_ts) {
    int err = 0;
    //lock the driver for bublishing
    ChaosLockGuard wl_io(mutex_publishing);
    //update infromation abour process
    updateProcInfo();
    
    //send datapack
    CDWShrdPtr data_pack = prepareNodeDataPack(*heath_set,
                                               publish_ts);
    if(data_pack.get()) {
      //  HM_DBG<<" HEALT DIRECT PACK:"<<data_pack->getCompliantJSONString();

      /*  err= DataManager::getInstance()->getDataLiveDriverNewInstance()->storeHealthData(heath_set->node_publish_key,
                                                                                          MOVE(data_pack),
                                                                                          DataServiceNodeDefinitionType::DSStorageTypeLive);
                                                                                          */
        //store data on cache
          BufferSPtr channel_data_injected(data_pack->getBSONDataBuffer().release());
        if( chaos::service_common::DriverPoolManager::getInstance()->getCacheDrvPtr()){
            chaos::service_common::DriverPoolManager::getInstance()->getCacheDrv().putData(heath_set->node_publish_key,channel_data_injected);
        } else {
            err = SharedManagedDirecIoDataDriver::getInstance()->getSharedDriver()->storeHealthData(heath_set->node_publish_key,
                                                                                          MOVE(data_pack),
                                                                                         DataServiceNodeDefinitionType::DSStorageTypeLive);
                                                                        
            if(err) {
                HM_ERR << "Error pushing health datapack for node:" << heath_set->node_uid << " with code:" << err;
            }
        }
    } else {
        HM_ERR << "Error allocating health datapack for node:" << heath_set->node_uid;
    }
}
        }}}