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

#include <chaos/common/configuration/GlobalConfiguration.h>
#include <chaos/common/utility/UUIDUtil.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/direct_io/DirectIODataPack.h>
#include <chaos/common/direct_io/impl/PSMDirectIOServer.h>
#include <boost/format.hpp>


#define PSMDIO_SRV_LOG_HEAD "["<<getName()<<"] - "

#define PSMS_LAPP LAPP_ << PSMDIO_SRV_LOG_HEAD
#define PSMDIO_SRV_LDBG_ LDBG_ << PSMDIO_SRV_LOG_HEAD
#define PSMDIO_SRV_LERR_ LERR_ << PSMDIO_SRV_LOG_HEAD


#define DIRECTIO_FREE_ANSWER_DATA(x)\
if(x && x->answer_data) free(x->answer_data);\
if(x) free(x);\
x = NULL;


namespace chaos_data = chaos::common::data;

using namespace chaos::common::direct_io::impl;
using namespace chaos::common::direct_io;

DEFINE_CLASS_FACTORY(PSMDirectIOServer, DirectIOServer);

PSMDirectIOServer::PSMDirectIOServer(std::string alias):
DirectIOServer(alias),
run_server(false),
direct_io_thread_number(2){};

PSMDirectIOServer::~PSMDirectIOServer(){};

//! Initialize instance
void PSMDirectIOServer::init(void *init_data)  {
    CDataWrapper *cfg = reinterpret_cast<CDataWrapper*>(init_data);
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
      throw chaos::CException(-3, "cannot initialize Publish Subscribe Consumer of topic:" + nodeuid  __PRETTY_FUNCTION__);
    }
    cons->addServer(msgbroker);
    prod->addServer(msgbroker);
    // subscribe to the queue of commands
    cons->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONARRIVE, boost::bind(&PSMDirectIOServer::messageHandler, this, _1));
    cons->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONERROR, boost::bind(&PSMDirectIOServer::messageError, this, _1));
    if (cons->applyConfiguration() != 0) {
        throw chaos::CException(-1, "cannot initialize Publish Subscribe:" + cons->getLastError(), __PRETTY_FUNCTION__);
    }
   
    } catch (std::exception& e) {
        throw CException(-2, e.what(), __PRETTY_FUNCTION__);
    } catch (...) {
        throw CException(-3, "generic error",  __PRETTY_FUNCTION__);
    }
    DirectIOServer::init(init_data);
    
    
}

//! Start the implementation
void PSMDirectIOServer::start()  {
    int err = 0;
    
    direct_io_thread_number = 1;
    DirectIOServer::start();
    
    PSMS_LAPP << "Subscribing to " << nodeuid + chaos::DataPackPrefixID::COMMAND_IO_POSTFIX;
    cons->subscribe(nodeuid + chaos::DataPackPrefixID::COMMAND_IO_POSTFIX);
    cons->start();
    prod->start();
}

//! Stop the implementation
void PSMDirectIOServer::stop()  {
    run_server = false;
    DirectIOServer::stop();
    
    cons->stop();
}

//! Deinit the implementation
void PSMDirectIOServer::deinit()  {
    //serverThreadGroup.stopGroup(true);
    
    DirectIOServer::deinit();
}

void PSMDirectIOServer::messageHandler( chaos::common::message::ele_t& data) {
    int64_t seq_id=-1;
    std::string src;
    //chaos::common::data::CDWUniquePtr data(d.cd.release());
    if(data.cd->hasKey(RPC_SEQ_KEY)){
        seq_id=data.cd->getInt64Value(RPC_SEQ_KEY);
    }
    if(data.cd->hasKey(RpcActionDefinitionKey::CS_CMDM_ANSWER_HOST_IP)){
        src=data.cd->getStringValue(RPC_SRC_UID);
    }
    PSMS_LDBG << "Message Received from node:"<<src<<" seq_id:"<<seq_id << " desc:"<<data.cd->getJSONString();
    CDWShrdPtr result_data_pack;

    if(data.cd->hasKey(RPC_SYNC_KEY) &&
        data.cd->getBoolValue(RPC_SYNC_KEY)) {
        
        result_data_pack = command_handler->executeCommandSync(MOVE(data.cd));
    } else {
        result_data_pack = command_handler->dispatchCommand(MOVE(data.cd));
    }

    if(result_data_pack.get() && src.size()){
        PSMS_LDBG << "Something to send back:"<<seq_id << "to node:"<<src<<" desc:"<<result_data_pack->getJSONString();
        prod->pushMsgAsync(*result_data_pack.get(),src);
    }
                    
}
void PSMDirectIOServer::messageError( chaos::common::message::ele_t& data) {
        PSMS_LERR  << "ERROR:";

}
void PSMDirectIOServer::worker(unsigned int w_type,
                               DirectIOHandlerPtr delegate) {
    int err = 0;
    
    std::string                 identity;
    void						*worker_socket          = NULL;
    bool						send_synchronous_answer = false;
    
    DirectIODataPackSPtr        data_pack_received;
    DirectIODataPackSPtr        data_pack_answer;
    
    MapPSMConfiguration         worker_empty_default_configuration;
    MapPSMConfiguration         worker_socket_configuration;
    worker_socket_configuration["PSM_LINGER"] = "500";
    worker_socket_configuration["PSM_RCVHWM"] = "1000";
    worker_socket_configuration["PSM_SNDHWM"] = "1000";
    worker_socket_configuration["PSM_RCVTIMEO"] = "-1";
    worker_socket_configuration["PSM_SNDTIMEO"] = "1000";
    
    if((worker_socket = zmq_socket(zmq_context,
                                   PSM_DEALER)) == NULL) {
        PSMDIO_SRV_LERR_ << "Error creating worker socket";
        return;
    }

    if((err = PSMBaseClass::configureSocketWithStartupParameter(worker_socket,
                                                                worker_socket_configuration,
                                                                worker_empty_default_configuration,
                                                                "PSM DirectIO Server worker"))){
        return;
    }
    
    if((err = PSMBaseClass::configureSocketWithStartupParameter(worker_socket,
                                                                worker_socket_configuration,
                                                                worker_empty_default_configuration,
                                                                "PSM DirectIO Server worker"))){
        return;
    }
    
    if(w_type == WorkerTypePriority) {
        if((err = PSMBaseClass::connectSocket(worker_socket,
                                              INPROC_PRIORITY,
                                              "PSM Server Worker"))) {
            PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error connecting worker socket with error %1%",%err);
            return;
        }
    } else if(w_type == WorkerTypeService) {
        if((err = PSMBaseClass::connectSocket(worker_socket,
                                              INPROC_SERVICE,
                                              "PSM Server Worker"))) {
            PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error connecting worker socket with error %1%",%err);
            return;
        }
    }
    
    PSMDIO_SRV_LDBG_ << "Entering in the thread loop for worker socket";
    while (run_server) {
        try {
            if((err = reveiceDatapack(worker_socket,
                                      identity,
                                      data_pack_received))) {
                data_pack_received.reset();
                continue;
            } else {
                //keep track if the cleint want the answer
                send_synchronous_answer = data_pack_received->header.dispatcher_header.fields.synchronous_answer;
                //call handler
                if((err = DirectIOHandlerPtrCaller(handler_impl, delegate)(MOVE(data_pack_received),
                                                                           data_pack_answer)) == 0) {
                    if(send_synchronous_answer &&
                       data_pack_answer) {
                        
                        if((err = sendDatapack(worker_socket,
                                               identity,
                                               MOVE(data_pack_answer)))){
                            PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error sending answer with code %1%", %err);
                        }
                    }
                } else {
                    PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error dispatching received message with code %1%", %err);
                }
            }
        } catch (CException& ex) {
            DECODE_CHAOS_EXCEPTION(ex)
        }
    }
    PSMDIO_SRV_LDBG_ << "Leaving the thread loop for worker socket";
    if((err = zmq_close(worker_socket))) {
        PSMDIO_SRV_LERR_ << CHAOS_FORMAT("Error closing worker socket with error %1%",%err);
    }
}
