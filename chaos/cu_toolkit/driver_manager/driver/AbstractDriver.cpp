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

#include <chaos/common/utility/UUIDUtil.h>
#include <string>

#include <chaos/cu_toolkit/driver_manager/driver/AbstractDriver.h>
#include <chaos/cu_toolkit/driver_manager/driver/DriverAccessor.h>

using namespace chaos::common::data;
using namespace chaos::common::utility;
//namespace chaos_thread_ns = chaos::common::thread;
using namespace chaos::cu::driver_manager::driver;

#define ADLAPP_ INFO_LOG_1_P(AbstractDriver, driver_uuid)
#define ADLDBG_ DBG_LOG_1_P(AbstractDriver, driver_uuid)
#define ADLERR_ ERR_LOG_1_P(AbstractDriver, driver_uuid)

/*------------------------------------------------------
 
 ------------------------------------------------------*/
AbstractDriver::AbstractDriver(BaseBypassShrdPtr custom_bypass_driver)
    : accessor_count(0), bypass_driver(MOVE(custom_bypass_driver)), o_exe(this), is_json_param(false), driver_need_to_deinitialize(false), driver_uuid(UUIDUtil::generateUUIDLite()), command_queue(new DriverQueueType()) {}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
AbstractDriver::~AbstractDriver() {}

// Initialize instance
void AbstractDriver::init(void *init_param) {
  driver_need_to_deinitialize = false;
  CDataWrapper parm;

  //!try to decode parameter string has json document
  is_json_param = parm.isJsonValue((const char *)init_param);

  ADLDBG_ << "Start in driver thread";
  //start interna thread for the waithing of the message
  thread_message_receiver.reset(new boost::thread(boost::bind(&AbstractDriver::scanForMessage, this)));

  //set the scheduler thread priority
#if defined(__linux__) || defined(__APPLE__)
  int                policy;
  struct sched_param param;
  pthread_t          threadID = (pthread_t)thread_message_receiver->native_handle();
  if (!pthread_getschedparam(threadID, &policy, &param)) {
    DEBUG_CODE(ADLAPP_ << "Default thread scheduler policy";)
    DEBUG_CODE(ADLAPP_ << "policy=" << ((policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???");)
    DEBUG_CODE(ADLAPP_ << "priority " << param.sched_priority;)

    policy               = SCHED_RR;
    param.sched_priority = sched_get_priority_max(SCHED_RR);
    if (!pthread_setschedparam(threadID, policy, &param)) {
      //successfull setting schedule priority to the thread
      DEBUG_CODE(ADLAPP_ << "new thread scheduler policy";)
      DEBUG_CODE(ADLAPP_ << "policy=" << ((policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???");)
      DEBUG_CODE(ADLAPP_ << "priority " << param.sched_priority;)
    }
  }
#endif
  ADLAPP_ << "Call custom driver initialization";

  DrvMsg              init_msg;
  int retry=3;
  ResponseMessageType id_to_read;
  AccessorQueueType   result_queue;
  init_msg.opcode        = OpcodeType::OP_INIT_DRIVER;
  init_msg.id            = 0;
  init_msg.inputData     = init_param;
  init_msg.drvResponseMQ = &result_queue;
  int ret;
  do{
  ret=command_queue->push(&init_msg,chaos::common::constants::CUTimersTimeoutinMSec);
    if(ret>=0){
      result_queue.wait_and_pop(id_to_read,chaos::common::constants::CUTimersTimeoutinMSec);
      if (init_msg.ret) {
        //in case we have error throw the exception
        throw CException(init_msg.ret, init_msg.err_msg, init_msg.err_dom);
      }
    }
  
  } while((ret<0) && (retry--));
}

// Deinit the implementation
void AbstractDriver::deinit() {
  ADLAPP_ << "Call custom driver deinitialization";
  // driverDeinit();
  DrvMsg              deinit_msg;
  ResponseMessageType id_to_read;
  AccessorQueueType   result_queue;

  //  std::memset(&deinit_msg, 0, sizeof(DrvMsg));
  deinit_msg.opcode        = OpcodeType::OP_DEINIT_DRIVER;
  deinit_msg.drvResponseMQ = &result_queue;
  //send opcode to driver implemetation
  driver_need_to_deinitialize = true;
  int ret=command_queue->push(&deinit_msg,chaos::common::constants::CUTimersTimeoutinMSec);
  if(ret>=0){
  //wait for completition
    result_queue.wait_and_pop(id_to_read,chaos::common::constants::CUTimersTimeoutinMSec);
  }
  //now join to  the thread if joinable
  if (thread_message_receiver->joinable()) {
    thread_message_receiver->join();
  }
}

const bool AbstractDriver::isDriverParamInJson() const {
  return is_json_param;
}

/*const Json::Value &AbstractDriver::getDriverParamJsonRootElement() const {
  return json_parameter_document;
}*/

/*------------------------------------------------------
 
 ------------------------------------------------------*/
bool AbstractDriver::getNewAccessor(DriverAccessor **newAccessor,const std::string& owner) {
  //allocate new accessor;
  DriverAccessor *result = new DriverAccessor(accessor_count++);
  if (result) {
    //set the parent uuid
    result->driver_uuid = driver_uuid;
    result->driverName  = driverName;
    result->owner.push_back(owner);
    result->command_queue = command_queue.get();
    boost::unique_lock<boost::shared_mutex> lock(accesso_list_shr_mux);
    accessors.push_back(result);
    lock.unlock();

    *newAccessor = result;
  }
  return result != NULL;
}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
bool AbstractDriver::releaseAccessor(DriverAccessor *accessor) {
  if (!accessor) {
    return false;
  }

  if (accessor->driver_uuid.compare(driver_uuid) != 0) {
    ADLERR_ << "has been requested to release an accessor with uuid=" << accessor->driver_uuid << "that doesn't belong to this driver with uuid =" << driver_uuid;
    return false;
  }
  boost::unique_lock<boost::shared_mutex> lock(accesso_list_shr_mux);

  accessors.erase(std::find(accessors.begin(), accessors.end(), accessor));
  lock.unlock();
  delete (accessor);

  return true;
}

/*------------------------------------------------------
 
 ------------------------------------------------------*/
void AbstractDriver::scanForMessage() {
  ADLAPP_ << "Scanner thread started for driver[" << driver_uuid << "]";
  MsgManagmentResultType::MsgManagmentResult opcode_submission_result = MsgManagmentResultType::MMR_ERROR;

  DrvMsgPtr current_message_ptr;
  int ret;
  do {
//        boost::unique_lock<boost::shared_mutex> lock(accesso_list_shr_mux);
    current_message_ptr=NULL;
    //wait for the new command
    ret=command_queue->wait_and_pop(current_message_ptr,chaos::common::constants::CUTimersTimeoutinMSec);
    if(ret<0){
        ADLDBG_ << "Scanner thread Timeout nothing received yed [" << driver_uuid << "]";
        continue;
    } 
    //check i opcode pointer is valid
    if (current_message_ptr == NULL) {
      continue;
    }

    try {
      //clean error message and domain

      //! check if we need to execute the private driver's opcode
      switch (current_message_ptr->opcode) {
        case OpcodeType::OP_GET_LASTERROR:
          current_message_ptr->resultData       = NULL;
          current_message_ptr->resultDataLength = 0;
          if(lastError.size()>0){
            current_message_ptr->resultData       = strdup(lastError.c_str());
            current_message_ptr->resultDataLength = lastError.size()+1;
            lastError=""; // clear last error
          }

        break;
        case OpcodeType::OP_SET_BYPASS:
          ADLDBG_ << "Switch to bypass driver";
          setBypass(true);
          break;
        case OpcodeType::OP_CLEAR_BYPASS:
          ADLDBG_ << "Switch to normal driver";
          setBypass(false);
          break;
        case OpcodeType::OP_INIT_DRIVER:
          opcode_submission_result = MsgManagmentResultType::MMR_EXECUTED;
          {
            ChaosUniquePtr<CDataWrapper> p;
            std::string                  init_paramter = static_cast<const char *>(current_message_ptr->inputData);
            int                          isjson        = 1;
            try {
              p      = CDataWrapper::instanceFromJson(init_paramter);
              isjson = (p->isEmpty() == false);
            } catch (...) {
              isjson = 0;
            }
            if (isjson) {
              ADLDBG_ << "JSON PARMS:" << p->getJSONString();
               if(p->hasKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DRIVER_PROP)&&p->isCDataWrapperValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DRIVER_PROP)){
                 CDataWrapper cd;
                 p->getCSDataValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DRIVER_PROP,cd);
                 importKeysAsProperties(cd);
            //      config.getCSDataValue(CAMERA_CUSTOM_PROPERTY,camera_custom_props);
                  ADLDBG_<<"driver properties"<<getProperties()->getJSONString();
        
                }
              driverInit(*p);
            } else {
              ADLDBG_ << "STRING PARMS:" << static_cast<const char *>(current_message_ptr->inputData);
              driverInit(static_cast<const char *>(current_message_ptr->inputData));
            }
          }

          break;

        case OpcodeType::OP_DEINIT_DRIVER:
          driverDeinit();
          opcode_submission_result = MsgManagmentResultType::MMR_EXECUTED;
          break;
        case OpcodeType::OP_GET_PROPERTIES: {
          chaos::common::data::CDWUniquePtr ret = getDrvProperties();
          int                               sizeb;
          current_message_ptr->resultData       = NULL;
          current_message_ptr->resultDataLength = 0;
          if (ret.get()) {
            const char *ptr = ret->getBSONRawData(sizeb);

            if ((sizeb > 0) && ptr) {
              current_message_ptr->resultData       = (char *)malloc(sizeb);
              current_message_ptr->resultDataLength = sizeb;
              memcpy(current_message_ptr->resultData, ptr, sizeb);
            }
          }
          break;
        }
        case OpcodeType::OP_SET_PROPERTIES: {
          chaos::common::data::CDWUniquePtr inp(new chaos::common::data::CDataWrapper());
          if(current_message_ptr->inputData){
            inp->setSerializedData((const char*)current_message_ptr->inputData);
            chaos::common::data::CDWUniquePtr ret   = setDrvProperties(inp);
            int                               sizeb = 0;
            const char *                      ptr   = NULL;
            if (ret.get()) {
              ptr = ret->getBSONRawData(sizeb);
            }
            current_message_ptr->resultData       = NULL;
            current_message_ptr->resultDataLength = 0;
            if ((sizeb > 0) && ptr) {
              current_message_ptr->resultData       = (char *)malloc(sizeb);
              current_message_ptr->resultDataLength = sizeb;
              memcpy(current_message_ptr->resultData, ptr, sizeb);
            }
          }
          break;
        }
        case OpcodeType::OP_SET_PROPERTY: {
          keyval_t *parms = (keyval_t *)current_message_ptr->inputData;
          if (parms && parms->key && parms->value) {
            current_message_ptr->ret = setDrvProperty(parms->key, parms->value);
          }
          break;
        }

        default: {
          //for custom opcode we call directly the driver implementation of execOpcode
          opcode_submission_result = o_exe->execOpcode(current_message_ptr);
          switch (opcode_submission_result) {
            case MsgManagmentResultType::MMR_ERROR:
              ADLERR_ << "an error has been returned by execOcode" << opcode_submission_result;

            case MsgManagmentResultType::MMR_EXECUTED:
              break;

            default:
              break;
          }
          break;
        }
      }
    } catch (CException &ex) {
      //chaos exception

      ADLERR_ << "an error has been catched code: " << ex.errorCode << " msg:" << ex.errorMessage.c_str() << " dom:" << ex.errorDomain.c_str() << " executing opcode:" << current_message_ptr->opcode;
      current_message_ptr->ret = ex.errorCode;
      strncpy(current_message_ptr->err_msg, ex.errorMessage.c_str(), DRVMSG_ERR_MSG_SIZE);
      strncpy(current_message_ptr->err_dom, ex.errorDomain.c_str(), DRVMSG_ERR_DOM_SIZE);
    } catch (std::exception &e) {
      opcode_submission_result = MsgManagmentResultType::MMR_ERROR;
      std::stringstream ss;
      ss << "Unexpected exception: \"" << e.what() << "\" executing opcode:" << current_message_ptr->opcode;
      ADLERR_ << ss.str();
      strncpy(current_message_ptr->err_msg, ss.str().c_str(), DRVMSG_ERR_MSG_SIZE);
      strncpy(current_message_ptr->err_dom, __PRETTY_FUNCTION__, DRVMSG_ERR_DOM_SIZE);
    } catch (...) {
      //unkonwn exception
      ADLERR_ << "an unknown exception executing opcode:" << current_message_ptr->opcode;
      opcode_submission_result = MsgManagmentResultType::MMR_ERROR;
      strncpy(current_message_ptr->err_msg, "Unexpected exception:", DRVMSG_ERR_MSG_SIZE);
      strncpy(current_message_ptr->err_dom, __PRETTY_FUNCTION__, DRVMSG_ERR_DOM_SIZE);
    }

    //notify the caller
    if (current_message_ptr->drvResponseMQ) {
      current_message_ptr->drvResponseMQ->push(current_message_ptr->id,chaos::common::constants::CUTimersTimeoutinMSec);
    }

  } while (current_message_ptr == NULL ||
           ((!driver_need_to_deinitialize)&&(current_message_ptr->opcode != OpcodeType::OP_DEINIT_DRIVER) ));
  ADLAPP_ << "Scanner thread terminated for driver[" << driver_uuid << "]";
}

void AbstractDriver::driverInit(const chaos::common::data::CDataWrapper &data) {
  ADLERR_ << "driver " << identification_string << " has json parameters you should implement driverInit(const chaos::common::data::CDataWrapper& data) initialization";
  driverInit(data.getCompliantJSONString().c_str());
}

void AbstractDriver::driverDeinit() {
  ADLDBG_ << "base driver " << identification_string <<" DEINIT";
}

const bool AbstractDriver::isBypass() const {
  return o_exe != this;
}

void AbstractDriver::setBypass(bool bypass) {
   // boost::unique_lock<boost::shared_mutex> lock(accesso_list_shr_mux);

  if (bypass) {
    LBypassDriverUnqPtrReadLock rl = bypass_driver.getReadLockObject();
    o_exe                          = bypass_driver().get();
  } else {
/*      DrvMsgPtr              msg;
    while(command_queue->pop(msg)){
        ADLDBG_ << "removing messages on bypass id:" << msg->id <<" message opcode:"<<msg->opcode;
    }*/
    o_exe = this;
  }
}

int AbstractDriver::setDrvProperty(const std::string &key, const std::string &value) {
  return 0;
}

chaos::common::data::CDWUniquePtr AbstractDriver::getDrvProperties() {

  return getProperties(true);
}
void AbstractDriver::setLastError(const std::string&str){
  lastError=str;
}

chaos::common::data::CDWUniquePtr AbstractDriver::setDrvProperties(chaos::common::data::CDWUniquePtr& drv) {
  if(drv.get()){
    return setProperties(*drv.get(),true);

  }
 return chaos::common::data::CDWUniquePtr();
}
MsgManagmentResultType::MsgManagmentResult AbstractDriver::execOpcode(DrvMsgPtr cmd){
  ADLERR_<<"OPCODE:"<<cmd->opcode<< " NOT IMPLEMENTED";
  return MsgManagmentResultType::MMR_ERROR;
}
