//
//  IODataDriver.cpp
//  CHAOSFramework
//
//  Created by Bisegni Claudio on 12/05/12.
//  Copyright (c) 2012 INFN. All rights reserved.
//

#include <chaos/common/global.h>
#include <chaos/common/io/IODataDriver.h>
#include <chaos/common/network/NetworkBroker.h>
#include <chaos/common/message/MDSMessageChannel.h>

#define IODataDriverLOG_HEAD "[IODataDriver] - "

#define IODataDriverLAPP	LAPP_ << IODataDriverLOG_HEAD
#define IODataDriverLDBG	LDBG_ << IODataDriverLOG_HEAD << __PRETTY_FUNCTION__ << " - "
#define IODataDriverLERR	LERR_ << IODataDriverLOG_HEAD << __PRETTY_FUNCTION__ << "(" << __LINE__ << ") - "

using namespace chaos;
using namespace chaos::common::io;
using namespace chaos::common::utility;
using namespace chaos::common::data;


/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODataDriver::init(void *init_parameter){
    
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void IODataDriver::deinit() {
    
}

int IODataDriver::removeData(const std::string& key,
                             uint64_t start_ts,
                             uint64_t end_ts) {
    return 0;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
ArrayPointer<CDataWrapper>*  IODataDriver::retriveData(const std::string& key, CDataWrapper*const) {

    //check for key length
    return retriveData(key);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
ArrayPointer<CDataWrapper>* IODataDriver::retriveData(const std::string& key)  {
  //  boost::mutex::scoped_lock l(iomutex);
    
    ArrayPointer<CDataWrapper> *result = new ArrayPointer<CDataWrapper>();
    
    CDWUniquePtr value=retrieveData(key);
    if (value.get()) {
        //some value has been received
        //allocate the data wrapper object with serialization got from memcached
        //CDataWrapper *dataWrapper =
        result->add(value.release());
    }
    return result;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
CDataWrapper* IODataDriver::updateConfiguration(CDataWrapper*){
    return NULL;
}
int IODataDriver::subscribe(const std::string&key){
    IODataDriverLERR << "Not implemented";
    return 0;

}
int IODataDriver::addHandler(const std::string &k,chaos::common::message::msgHandler cb){
    IODataDriverLERR << "Not implemented";

    return 0;
}
int IODataDriver::addHandler(chaos::common::message::msgHandler cb){
    IODataDriverLERR << "Not implemented";

    return 0;
}

int IODataDriver::loadDatasetFromSnapshot(const std::string& restore_point_tag_name,
                                                           const std::string& key,
                                                           chaos_data::CDWShrdPtr& cdw_shrd_ptr){
                                                                //return IODirectIODriver::loadDatasetTypeFromSnapshotTag(restore_point_tag_name,key,dataset_type,cdw_shrd_ptr);                                                  
    chaos::common::data::CDataWrapper data_set;
    int err = chaos::common::network::NetworkBroker::getInstance()->getMetadataserverMessageChannel()->loadSnapshotNodeDataset(restore_point_tag_name,key,data_set);
   // IODirectIOPSMsgDriver_DLDBG_<<"SNAPSHOT:"<<data_set.getJSONString();
    if((!err)){
       cdw_shrd_ptr.reset(data_set.clone().release());

    }
    
    return err;

}
