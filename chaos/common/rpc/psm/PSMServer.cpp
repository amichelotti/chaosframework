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
#include <chaos/common/chaos_constants.h>
#include <chaos/common/exception/exception.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/rpc/RpcServerHandler.h>
#include <chaos/common/utility/TimingUtil.h>
#define PSMS_LAPP INFO_LOG(PSMServer)
#define PSMS_LDBG DBG_LOG(PSMServer)
#define PSMS_LERR ERR_LOG(PSMServer)

using namespace std;
using namespace chaos;
using namespace boost;
using namespace chaos::common::data;
#include "PSMServer.h"

DEFINE_CLASS_FACTORY(PSMServer, RpcServer);
PSMServer::PSMServer(const string& alias):
RpcServer(alias){
    is_psm=true;
    
}

PSMServer::~PSMServer() {
    
}

//init the server getting the configuration value
void PSMServer::init(void *init_data) {
    RpcServer::init(init_data);
    PSMS_LAPP << "initialization";
   try{
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
    std::string gname;
    if(cfg->hasKey(InitOption::OPT_GROUP_NAME)){
        gname=cfg->getStringValue(InitOption::OPT_GROUP_NAME);
        PSMS_LAPP << "belong to group:\""<<gname<<"\"";

    } else {
        gname=nodeuid;
    }

    cons = chaos::common::message::MessagePSDriver::getNewConsumerDriver(msgbrokerdrv, gname);
    prod = chaos::common::message::MessagePSDriver::getProducerDriver(msgbrokerdrv);

    if (cons.get() == 0) {
      throw chaos::CException(-3, "cannot initialize Publish Subscribe Consumer of topic:" + nodeuid + "_cmd", __PRETTY_FUNCTION__);
    }
    cons->addServer(msgbroker);
    prod->addServer(msgbroker);
    // subscribe to the queue of commands
    cons->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONARRIVE, boost::bind(&PSMServer::messageHandler, this, _1));
    cons->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONERROR, boost::bind(&PSMServer::messageError, this, _1));
    cons->setOption("allow.auto.create.topics","true");
    if (cons->applyConfiguration() != 0) {
        throw chaos::CException(-1, "cannot initialize Publish Subscribe:" + cons->getLastError(), __PRETTY_FUNCTION__);
    }

  
    } catch (std::exception& e) {
        throw CException(-2, e.what(), "PSMServer::init");
    } catch (...) {
        throw CException(-3, "generic error", "PSMServer::init");
    }
}
void PSMServer::messageHandler( chaos::common::message::ele_t& data) {
    int64_t seq_id=-1,ts=0;
    int64_t now=(int64_t)chaos::common::utility::TimingUtil::getTimeStamp();
    std::string src;
    //chaos::common::data::CDWUniquePtr data(d.cd.release());
    if(data.cd->hasKey(RPC_SEQ_KEY)){
        seq_id=data.cd->getInt64Value(RPC_SEQ_KEY);
    }
    if(data.cd->hasKey(RPC_TS_KEY)){
        ts=data.cd->getInt64Value(RPC_TS_KEY);
        if(((int64_t)(now-ts))>((int64_t)(4*chaos::common::constants::CUTimersTimeoutinMSec))){
            PSMS_LERR << "discarding Message TOO OLD Received from node:"<<src<<" seq_id:"<<seq_id<<" sent:"<<(now-ts)*1.0/1000.0<<" s ("<<ts<<" - "<<chaos::common::utility::TimingUtil::toString(ts)<<")";//<< " desc:"<<data.cd->getJSONString();
            return;
        }   
    }

    if(data.cd->hasKey(RpcActionDefinitionKey::CS_CMDM_ANSWER_HOST_IP)){
        src=data.cd->getStringValue(RPC_SRC_UID);
    }
    if(data.cd->hasKey(RpcActionDefinitionKey::CS_CMDM_MESSAGE_ID)){
        PSMS_LDBG << data.cd->getInt32Value(RpcActionDefinitionKey::CS_CMDM_MESSAGE_ID)<<" - Message Received from node:"<<src<<" seq_id:"<<seq_id ;//<< " desc:"<<data.cd->getJSONString();

    } else {
        PSMS_LDBG << "Message Received from node:"<<src<<" seq_id:"<<seq_id<<" sent:"<<(now-ts)<<" ms";//<< " desc:"<<data.cd->getJSONString();

    }

    CDWShrdPtr result_data_pack;

    if(data.cd->hasKey(RPC_SYNC_KEY) &&
        data.cd->getBoolValue(RPC_SYNC_KEY)) {
        
        result_data_pack = command_handler->executeCommandSync(MOVE(data.cd));
    } else {
        PSMS_LDBG<<"dispatch "<<data.cd->getJSONString();
        result_data_pack = command_handler->dispatchCommand(MOVE(data.cd));
    }

    if(result_data_pack.get() && src.size()){
     //   PSMS_LDBG << "Something to send back:"<<seq_id << "to node:"<<src;
        if(prod->pushMsgAsync(*result_data_pack.get(),src)==false){
                 PSMS_LERR << "Error sending packet back:"<<result_data_pack->getJSONString();

        } 
        if(prod->getMsgOpt()!=chaos::common::message::MessagePublishSubscribeBase::MSG_SYNCH){

            prod->flush(1000);
        }
        
    }
                    
}
void PSMServer::messageError( chaos::common::message::ele_t& data) {
        PSMS_LERR  << "ERROR:";

}
 std::string PSMServer::getPublishedEndpoint(){
    
    return nodeuid + chaos::DataPackPrefixID::COMMAND_DATASET_POSTFIX;
}
//start the rpc adapter
void PSMServer::start() {
    if(cfg->hasKey("ismds")){
        PSMS_LAPP << "Subscribing to " <<chaos::common::constants::CHAOS_ADMIN_ADMIN_TOPIC;
        cons->subscribe(chaos::common::constants::CHAOS_ADMIN_ADMIN_TOPIC);
    }
    PSMS_LAPP << "Subscribing to " << nodeuid + chaos::DataPackPrefixID::COMMAND_DATASET_POSTFIX;
    cons->subscribe(nodeuid + chaos::DataPackPrefixID::COMMAND_DATASET_POSTFIX);
    cons->start();
    prod->start();
}

//start the rpc adapter
void PSMServer::stop() {
    cons->stop();
    prod->stop();
    
}

//deinit the rpc adapter
void PSMServer::deinit() {
    
    PSMS_LAPP << "PSMServer deinit";
}
