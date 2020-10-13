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
DriverAccessor::~DriverAccessor() {}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
bool DriverAccessor::send(DrvMsgPtr cmd,
                          uint32_t  inc_priority) {
    CHAOS_ASSERT(cmd)
    ResponseMessageType answer_message = 0;
    //fill the cmd with the information for retrieve it
    cmd->id            = messages_count++;
    cmd->drvResponseMQ = &accessor_sync_mq;
    cmd->device_param = device_param;
    //send command
    command_queue->push(cmd, base_opcode_priority + inc_priority);
    //whait the answer
    accessor_sync_mq.wait_and_pop(answer_message);
    if((*cmd->err_msg!=0) && (*cmd->err_dom!=0)&& (cmd->ret!=0)){
        LDBG_<<"Launch Exception msg:"<<cmd->err_msg<<" dom:"<<cmd->err_dom<<" ret:"<<cmd->ret;
        throw chaos::CFatalException(cmd->ret,cmd->err_msg,cmd->err_dom);
    }
    //check result
    return (answer_message == MsgManagmentResultType::MMR_EXECUTED);
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
    command_queue->push(cmd, base_opcode_priority + inc_priority);
    return true;
}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
bool DriverAccessor::getLastAsyncMsg(ResponseMessageType& message_id) {
    return accessor_sync_mq.try_pop(message_id);
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


