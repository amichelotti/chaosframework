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
#include <chaos/common/utility/UUIDUtil.h>

#include <chaos/cu_toolkit/driver_manager/driver/AbstractDriver.h>
#include <chaos/cu_toolkit/driver_manager/driver/DriverAccessor.h>
#include "../../windowsCompliant.h"
using namespace chaos::cu::driver_manager::driver;

#define DALAPP_		INFO_LOG(DriverAccessor)
#define DALDBG_		DBG_LOG(DriverAccessor)
#define DALERR_		ERR_LOG(DriverAccessor)

/*------------------------------------------------------
 
 ------------------------------------------------------*/
DriverAccessor::DriverAccessor(unsigned int _accessor_index):
accessor_index(_accessor_index),
messages_count(0),
accessor_async_mq(),
accessor_sync_mq(),
command_queue(NULL),
base_opcode_priority(0) {}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
DriverAccessor::~DriverAccessor() {

}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
uint64_t DriverAccessor::getMessageCount(){
    return messages_count;

}

bool DriverAccessor::send(DrvMsgPtr cmd,
                          uint32_t  timeout_ms) {
    //static int counter=0;
    CHAOS_ASSERT(cmd)
    ResponseMessageType answer_message = 0;
    {
        //boost::unique_lock<boost::shared_mutex> lock(mutex_queue);

    //fill the cmd with the information for retrieve it
        cmd->id            = messages_count++;
        cmd->drvResponseMQ = &accessor_sync_mq;
        cmd->device_param = device_param;
        *cmd->err_msg=0;
        *cmd->err_dom=0;
        cmd->ret=0;
        //send command
       // command_queue->push(cmd, base_opcode_priority + inc_priority);
        command_queue->push(cmd);
    //DALDBG_<<owner<<" ["<<counter<<"] send opcode:"<<cmd->opcode;
    }
    //wait the answer
 //   DALDBG_<<owner[0]<<" ["<<cmd->id<<"] send opcode:"<<cmd->opcode<<" timeout:"<<timeout_ms;

    int ret=accessor_sync_mq.wait_and_pop(answer_message,timeout_ms);
    if(ret<0){
        std::stringstream ss;
        ss<<cmd->id<<","<<accessor_sync_mq.length()<<"] Timeout of:"<<timeout_ms<<" ms expired, executing opcode:"<<cmd->opcode;
       // throw chaos::CFatalException(ret,ss.str(),__FUNCTION__);
        DALERR_<<ss.str();
        strncpy(cmd->err_msg,ss.str().c_str(),DRVMSG_ERR_MSG_SIZE);
        strncpy(cmd->err_dom,__FUNCTION__,DRVMSG_ERR_MSG_SIZE);
        cmd->ret=ret;
        return false;
    } 
    if((*cmd->err_msg!=0) && (*cmd->err_dom!=0)&& (cmd->ret!=0)){
        DALERR_<<"Launch Exception msg:"<<cmd->err_msg<<" dom:"<<cmd->err_dom<<" ret:"<<cmd->ret;
        throw chaos::CFatalException(cmd->ret,cmd->err_msg,cmd->err_dom);
    }
    
    //check result
    return (answer_message == MsgManagmentResultType::MMR_EXECUTED);
}
int DriverAccessor::stop(){
    DALDBG_<<" Stopping:'"<<driverName<<"' signaling queues";
    accessor_sync_mq.unblock();
    return 0;
}
int DriverAccessor::start(){
    return 0;

}
/*------------------------------------------------------
 
 ------------------------------------------------------*/
bool DriverAccessor::sendAsync(DrvMsgPtr cmd, ResponseMessageType& message_id, uint32_t inc_priority) {
    CHAOS_ASSERT(cmd)
    
    //fill the cmd with the information for retrive it
    cmd->id = message_id = messages_count++;
    cmd->drvResponseMQ   = &accessor_async_mq;
    cmd->device_param = device_param;
    //send message
  //  command_queue->push(cmd, base_opcode_priority + inc_priority);
  command_queue->push(cmd);
    return true;
}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
bool DriverAccessor::getLastAsyncMsg(ResponseMessageType& message_id) {
    return accessor_sync_mq.pop(message_id);
}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
bool DriverAccessor::operator==(const DriverAccessor& a) {
    return this->accessor_index = a.accessor_index;
}
namespace chaos_driver=::chaos::cu::driver_manager::driver;
using namespace chaos::cu::driver_manager::driver;

std::string DriverAccessor::getUID() const {
    return driver_uuid;
}
std::string DriverAccessor::getDriverName() const{
    return driverName;
}

chaos::common::data::CDWUniquePtr DriverAccessor::getDrvProperties(){
    chaos_driver::DrvMsg message;
    message.opcode=OpcodeType::OP_GET_PROPERTIES;
    send(&message);

    if(message.resultData && message.resultDataLength){
        chaos::common::data::CDWUniquePtr ptr(new chaos::common::data::CDataWrapper());
        ptr->setSerializedData((const char*)message.resultData);
        free(message.resultData);
        return ptr;
    }
    return chaos::common::data::CDWUniquePtr();
}
chaos::common::data::CDWUniquePtr DriverAccessor::setDrvProperties(chaos::common::data::CDWUniquePtr& data){
    chaos_driver::DrvMsg message;
    message.opcode=OpcodeType::OP_SET_PROPERTIES;
    int sizeb;
    chaos::common::data::CDWUniquePtr tmp;
    if(data.get()){
          tmp=data->clone();
          char*ptr=(char*)tmp->getBSONRawData(sizeb);
          message.inputData=ptr;
          message.inputDataLength=sizeb;
          send(&message);

    if(message.resultData && message.resultDataLength){
        chaos::common::data::CDWUniquePtr ptr(new chaos::common::data::CDataWrapper((const char*)message.resultData));
        free(message.resultData);
        return ptr;
    }
    }
    return chaos::common::data::CDWUniquePtr(); 
}

int DriverAccessor::setDrvProperty(const std::string& key, const std::string& value){
    chaos_driver::DrvMsg message;

    keyval_t args;
    args.key=(key.c_str());
    args.value=(value.c_str());
    message.opcode=OpcodeType::OP_SET_PROPERTY;
    message.inputData=&args;
    message.inputDataLength=sizeof(keyval_t);
    send(&message);
    return message.ret;

}

std::string DriverAccessor::getLastError(){
     chaos_driver::DrvMsg message;

    message.opcode=OpcodeType::OP_GET_LASTERROR;
    send(&message);
    if(message.resultData && message.resultDataLength){
        std::string result((const char*)message.resultData,message.resultDataLength);
        free(message.resultData);
        return result;
    }
    return std::string();
 
}


