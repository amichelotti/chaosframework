/*
 *	SnapshotDataAccess.cpp
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

#include "SnapshotDataAccess.h"
#include "DataServiceDataAccess.h"

#include <chaos/common/io/IODirectIODriver.h>
#include <chaos/common/network/NetworkBroker.h>
#include <chaos/common/chaos_types.h>

#include <boost/format.hpp>

using namespace chaos::common::io;
using namespace chaos::common::utility;
using namespace chaos::common::network;
using namespace chaos::metadata_service::persistence::data_access;

#define SDA_INFO INFO_LOG(SnapshotDataAccess)
#define SDA_DBG  DBG_LOG(SnapshotDataAccess)
#define SDA_ERR  ERR_LOG(SnapshotDataAccess)

CHAOS_DEFINE_VECTOR_FOR_TYPE(boost::shared_ptr<chaos::common::data::CDataWrapper>, CDSList)

DEFINE_DA_NAME(SnapshotDataAccess)

SnapshotDataAccess::SnapshotDataAccess(DataServiceDataAccess *_data_service_da):
AbstractDataAccess("SnapshotDataAccess"),
data_service_da(_data_service_da){}

SnapshotDataAccess::~SnapshotDataAccess() {}

int SnapshotDataAccess::createNewSnapshot(const std::string& snapshot_name,
                                          const ChaosStringVector& node_uid_list) {
    
    std::auto_ptr<IODataDriver> data_driver(ObjectFactoryRegister<IODataDriver>::getInstance()->getNewInstanceByName("IODirectIODriver"));
    if(data_driver.get() == NULL) return -1;
    
    int err = 0;
    CDSList best_cds;
    //set the information
    IODirectIODriverInitParam init_param;
    std::memset(&init_param, 0, sizeof(IODirectIODriverInitParam));
    //get client and endpoint
    init_param.network_broker = NetworkBroker::getInstance();
    init_param.client_instance = NULL;
    init_param.endpoint_instance = NULL;
    ((IODirectIODriver*)data_driver.get())->setDirectIOParam(init_param);
    try{
        data_driver->init(NULL);
        
        data_service_da->getBestNDataService(best_cds, 3);
        
        for(CDSListIterator it = best_cds.begin();
            it != best_cds.end();
            it++) {
            if(!(*it)->hasKey(chaos::NodeDefinitionKey::NODE_DIRECT_IO_ADDR) || !(*it)->hasKey(chaos::DataServiceNodeDefinitionKey::DS_DIRECT_IO_ENDPOINT)) continue;
            
            //add endpoint
            const std::string endpoint_addr =  boost::str(boost::format("%1%|%2%")%
                                                          (*it)->getStringValue(chaos::NodeDefinitionKey::NODE_DIRECT_IO_ADDR)%
                                                          (*it)->getInt32Value(chaos::DataServiceNodeDefinitionKey::DS_DIRECT_IO_ENDPOINT));
            ((IODirectIODriver*)data_driver.get())->addServerURL(endpoint_addr);
        }
        
        err = data_driver->createNewSnapshot(snapshot_name,
                                             node_uid_list);
    } catch(CException& ex) {
        err = ex.errorCode;
    } catch(...) {
        err = -10000;
    }
    data_driver->deinit();
    
    return err;
}


int SnapshotDataAccess::getNodeInSnapshot(const std::string& snapshot_name,
                                          ChaosStringVector& node_in_snapshot) {
    return 0;
}

int SnapshotDataAccess::getSnapshotForNode(const std::string& node_unique_id,
                                           ChaosStringVector& snapshot_for_node) {
    return 0;
}

int SnapshotDataAccess::deleteSnapshot(const std::string& snapshot_name) {
    std::auto_ptr<IODataDriver> data_driver(ObjectFactoryRegister<IODataDriver>::getInstance()->getNewInstanceByName("IODirectIODriver"));
    if(data_driver.get() == NULL) return -1;
    
    int err = 0;
    CDSList best_cds;
    //set the information
    IODirectIODriverInitParam init_param;
    std::memset(&init_param, 0, sizeof(IODirectIODriverInitParam));
    //get client and endpoint
    init_param.network_broker = NetworkBroker::getInstance();
    init_param.client_instance = NULL;
    init_param.endpoint_instance = NULL;
    ((IODirectIODriver*)data_driver.get())->setDirectIOParam(init_param);
    try{
        data_driver->init(NULL);
        
        data_service_da->getBestNDataService(best_cds, 3);
        
        for(CDSListIterator it = best_cds.begin();
            it != best_cds.end();
            it++) {
            if(!(*it)->hasKey(chaos::NodeDefinitionKey::NODE_DIRECT_IO_ADDR) || !(*it)->hasKey(chaos::DataServiceNodeDefinitionKey::DS_DIRECT_IO_ENDPOINT)) continue;
            
            //add endpoint
            const std::string endpoint_addr =  boost::str(boost::format("%1%|%2%")%
                                                          (*it)->getStringValue(chaos::NodeDefinitionKey::NODE_DIRECT_IO_ADDR)%
                                                          (*it)->getInt32Value(chaos::DataServiceNodeDefinitionKey::DS_DIRECT_IO_ENDPOINT));
            ((IODirectIODriver*)data_driver.get())->addServerURL(endpoint_addr);
        }
        
        err = data_driver->deleteSnapshot(snapshot_name);
    } catch(CException& ex) {
        err = ex.errorCode;
    } catch(...) {
        err = -10000;
    }
    data_driver->deinit();
    
    return err;
    
}
