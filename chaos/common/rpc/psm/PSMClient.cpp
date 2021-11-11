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

#include <chaos/common/global.h>
#include <chaos/common/rpc/ChaosRpc.h>
#include <chaos/common/async_central/AsyncCentralManager.h>
#include <chaos/common/chaos_constants.h>
#include <chaos/common/configuration/GlobalConfiguration.h>
#include <chaos/common/async_central/AsyncCentralManager.h>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <chaos/common/chaos_errors.h>

using namespace chaos;
using namespace chaos::common::data;
using namespace std;
using namespace boost;
using namespace boost::algorithm;
#include "PSMClient.h"
#define PSMC_LAPP INFO_LOG(PSMClient)
#define PSMC_LDBG DBG_LOG(PSMClient)
#define PSMC_LERR ERR_LOG(PSMClient)

#define PSM_DO_AGAIN(x) do{x}while(err == EAGAIN);
#define PSM_SOCKET_MAINTENANCE_TIMEOUT (5000 * 30)
#define PSM_SOCKET_LIFETIME_TIMEOUT (5000 * 60)
//-------------------------------------------------------
DEFINE_CLASS_FACTORY(PSMClient, RpcClient);

PSMClient::PSMClient(const string& alias):
RpcClient(alias){    
    seq_id=0;
    is_psm=true;

}

PSMClient::~PSMClient(){
//    #ifndef CHAOS_PROMETHEUS
//    delete counter_zmqerror_uptr;
//    #endif
}

/*
 Initialization method for output buffer
 */
void PSMClient::init(void *init_data) {
    CDataWrapper *cfg = reinterpret_cast<CDataWrapper*>(init_data);
    seq_id=0;
    PSMC_LAPP << "initialization";
    if(!cfg->hasKey(InitOption::OPT_MSG_BROKER_SERVER)){
        throw chaos::CException(-1, "a not empty broker must be given", __PRETTY_FUNCTION__);
    }
    if(!cfg->hasKey(chaos::InitOption::OPT_NODEUID)){
      throw chaos::CException(-1, "a not empty and unique id must be given", __PRETTY_FUNCTION__);
    }
    nodeuid      = cfg->getStringValue(chaos::InitOption::OPT_NODEUID);
    std::string msgbrokerdrv = "kafka-rdk";
    if(cfg->hasKey(InitOption::OPT_MSG_BROKER_DRIVER)){
        msgbrokerdrv     = cfg->getStringValue(chaos::InitOption::OPT_MSG_BROKER_DRIVER);

    }
    
    std::string msgbroker = cfg->getStringValue(InitOption::OPT_MSG_BROKER_SERVER);

    prod = chaos::common::message::MessagePSDriver::getProducerDriver(msgbrokerdrv);
        PSMC_LAPP << "Initializing producer based on " << msgbroker<<" ("+msgbrokerdrv+")";

    prod->addServer(msgbroker);

    if (prod->applyConfiguration() != 0) {
      throw chaos::CException(-2, "cannot initialize Publish Subscribe Producer:" + prod->getLastError(), __PRETTY_FUNCTION__);
    }

}

/*
 start the rpc adapter
 */
void PSMClient::start() {
    int err = 0;
    prod->start();

    //start timere after and repeat every one minut
  //  if((err = chaos::common::async_central::AsyncCentralManager::getInstance()->addTimer(this, PSM_SOCKET_MAINTENANCE_TIMEOUT, PSM_SOCKET_MAINTENANCE_TIMEOUT))) {LOG_AND_TROW(PSMC_LERR, err, "Error adding timer")}
}

/*
 start the rpc adapter
 */
void PSMClient::stop() {
    int err = 0;
    prod->stop();
    //  if((err = chaos::common::async_central::AsyncCentralManager::getInstance()->removeTimer(this))) {LOG_AND_TROW(PSMC_LERR, err, "Error removing timer")}
}

/*
 Deinitialization method for output buffer
 */
void PSMClient::deinit() {
   
    PSMC_LAPP << "PSM Destroyed";

}

/*
 
 */
bool PSMClient::submitMessage(NFISharedPtr forwardInfo,
                              bool onThisThread) {
    CHAOS_ASSERT(forwardInfo);
    ElementManagingPolicy ePolicy;
    try{
        forwardInfo->is_psm=true;
        if(!forwardInfo->destinationAddr.size())
            throw CException(0, "No destination in message description", __PRETTY_FUNCTION__);
        if(!forwardInfo->hasMessage())
            throw CException(0, "No message in description", __PRETTY_FUNCTION__);
        if(forwardInfo->message->hasKey(RpcActionDefinitionKey::CS_CMDM_ANSWER_HOST_IP)){
            forwardInfo->message->removeKey(RpcActionDefinitionKey::CS_CMDM_ANSWER_HOST_IP);
            forwardInfo->message->addStringValue(RpcActionDefinitionKey::CS_CMDM_ANSWER_HOST_IP,nodeuid+chaos::DataPackPrefixID::COMMAND_DATASET_POSTFIX);
        }
        forwardInfo->message->addBoolValue(RPC_SYNC_KEY, RpcClient::syncrhonous_call);
        forwardInfo->message->addInt64Value(RPC_SEQ_KEY, (++seq_id));
      
        std::string key=forwardInfo->destinationAddr;
        PSMC_LDBG<<seq_id<<"] Reqid:"<<forwardInfo->sender_request_id<<" "<<forwardInfo->sender_node_id<<" Sends message to:"<<forwardInfo->destinationAddr<<" size:"<<forwardInfo->message->getBSONRawSize();
        prod->pushMsgAsync(*forwardInfo->message.get(),key);

    } catch(CException& ex){
        //in this case i need to delete the memory
        //in this case i need to delete te memory allocated by message
        DECODE_CHAOS_EXCEPTION(ex)
    }
    return true;
}

//-----timer handler------
void PSMClient::timeout() {
   
}



uint64_t PSMClient::getMessageQueueSize() {
    return 0;
}
