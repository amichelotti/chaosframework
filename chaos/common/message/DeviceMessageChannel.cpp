/*
 *	DeviceMessageChannel.cpp
 *	!CHAOS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2012 INFN, National Institute of Nuclear Physics
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
#include "DeviceMessageChannel.h"
#include <chaos/common/chaos_constants.h>

using namespace chaos::common::message;
using namespace chaos::common::data;

#define DMCINFO INFO_LOG(DeviceMessageChannel)
#define DMCDBG DBG_LOG(DeviceMessageChannel)
#define DMCERR ERR_LOG(DeviceMessageChannel)


    //------------------------------------
DeviceMessageChannel::DeviceMessageChannel(NetworkBroker *msg_broker,
                                           CDeviceNetworkAddress *_device_network_address,
                                           MessageRequestDomainSHRDPtr _new_message_request_domain):
NodeMessageChannel(msg_broker,
                   _device_network_address,
                   _new_message_request_domain),
device_network_address(_device_network_address),
local_mds_channel(NULL),
online(false),
auto_reconnection(false){}

/*!
 Initialization phase of the channel
 */
void DeviceMessageChannel::init() throw(CException) {
    DMCINFO<< "Try to allocate local mds channel";
    if((local_mds_channel = getBroker()->getMetadataserverMessageChannel(getMessageRequestDomain())) == NULL) {
        throw CException(-1, "Local metadata server channel not found", __PRETTY_FUNCTION__);
    }
    DMCINFO<< "Local MDS Channel allocated";

    if(device_network_address->ip_port.size() ==0 ||
       device_network_address->node_id.size() == 0) {
            //force to update the network id
        tryToReconnect();
    }
}

/*!
 Initialization phase of the channel
 */
void DeviceMessageChannel::deinit() throw(CException) {
    if(local_mds_channel) {
        DMCINFO<< "Dispose local mds channel";
        getBroker()->disposeMessageChannel(local_mds_channel);
        local_mds_channel = NULL;
    }
}

#pragma health utility
void DeviceMessageChannel::setNewAddress(CDeviceNetworkAddress *new_device_network_address) {
    NodeMessageChannel::setNewAddress(new_device_network_address);
    device_network_address = new_device_network_address;
    DMCINFO << boost::str(boost::format("New netowrk address for device %1% has been set[%2%/%3%]")%device_network_address->device_id%device_network_address->node_id%device_network_address->ip_port);
}

void DeviceMessageChannel::timeout() {
    DMCINFO << boost::str(boost::format("Automatic reconnection for device %1% on [%2%/%3%]")%device_network_address->device_id%device_network_address->node_id%device_network_address->ip_port);
    tryToReconnect();
}

bool DeviceMessageChannel::udpateNetworkAddress(int32_t millisec_to_wait) {
    int err = ErrorCode::EC_NO_ERROR;
    if(local_mds_channel == NULL) return false;
    DMCINFO << "Update netowrk address for device " <<device_network_address->device_id;
    CDeviceNetworkAddress *tmp_addr = NULL;
        //ask to mds the new network address of the node
    if((err = local_mds_channel->getNetworkAddressForDevice(device_network_address->device_id, &tmp_addr))){
            //we have had error on comunication
        DMCINFO << "Error updating network address for " << device_network_address->node_id;
        return false;
    } else if (tmp_addr == NULL) {
        DMCINFO << "No network address received for " << device_network_address->node_id;
        return false;
    }

        //we can proceed
    std::auto_ptr<CDeviceNetworkAddress> new_device_network_address(tmp_addr);

    CHAOS_ASSERT((new_device_network_address->device_id.compare(device_network_address->device_id) == 0));

        //check if something has changed
    bool node_id_changed = (new_device_network_address->node_id.compare(device_network_address->node_id) != 0);
    bool node_ip_changed = (new_device_network_address->ip_port.compare(device_network_address->ip_port) != 0);
    bool is_changed = node_id_changed || node_ip_changed;
    if(is_changed) {
            //release new address to this channel
        setNewAddress(new_device_network_address.release());
    }
    return is_changed;
}

void DeviceMessageChannel::setOnline(bool new_online_state) {
    if(online == new_online_state) return;

    if(new_online_state == false) {
        online = new_online_state;
        DMCINFO << "Device " <<device_network_address->device_id<< " is gone offline";
            //enable auto reconnection if we need
        if(auto_reconnection) {async_central::AsyncCentralManager::getInstance()->addTimer(this, 1000, 1000);}
    } else {
        DMCINFO << "Device " <<device_network_address->device_id<< " is respawned";
            //disable auto reconnection
        async_central::AsyncCentralManager::getInstance()->removeTimer(this);

        online = new_online_state;
    }
}

void DeviceMessageChannel::tryToReconnect() {
        //!in this case we need to check on mds if somethig is changed
    if(online == false ){
        udpateNetworkAddress(5000);
            //try to check on device if it is online
        std::auto_ptr<MessageRequestFuture> check_rpc_for_dev = checkRPCInformation();
        if(check_rpc_for_dev->wait(5000)) {
            if(check_rpc_for_dev->getResult() &&
               check_rpc_for_dev->getResult()->hasKey("alive")) {
                setOnline(check_rpc_for_dev->getResult()->getBoolValue("alive"));
            } else {
                setOnline(false);
            }
        }
    }
}

bool DeviceMessageChannel::isOnline() {
    return online;
}

void DeviceMessageChannel::setAutoReconnection(bool _auto_reconnection) {
    if((auto_reconnection = _auto_reconnection)){
        if(online == false) {async_central::AsyncCentralManager::getInstance()->addTimer(this, 1000, 1000);}
    } else {
        async_central::AsyncCentralManager::getInstance()->removeTimer(this);
    }
}

#pragma device methods
int DeviceMessageChannel::recoverDeviceFromError(int32_t millisec_to_wait){
    CDataWrapper message_data;
    message_data.addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, device_network_address->device_id);
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              NodeDomainAndActionRPC::ACTION_NODE_RECOVER,
                                              &message_data,
                                              millisec_to_wait));
        //CHECK_TIMEOUT_AND_RESULT_CODE(result, err)
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::initDevice(CDataWrapper *initData, int32_t millisec_to_wait) {
        //int err = ErrorCode::EC_NO_ERROR;
    CHAOS_ASSERT(initData)
    auto_ptr<CDataWrapper> initResult(sendRequest(device_network_address->node_id,
                                                  NodeDomainAndActionRPC::ACTION_NODE_INIT,
                                                  initData,
                                                  millisec_to_wait));
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}


int DeviceMessageChannel::initDeviceToDefaultSetting(int32_t millisec_to_wait) {
    int err = ErrorCode::EC_NO_ERROR;
    CDataWrapper *tmp_cdw_ptr = NULL;
    if(local_mds_channel == NULL) return -100;
    if((err = local_mds_channel->getLastDatasetForDevice(device_network_address->device_id, &tmp_cdw_ptr))){
            //we have had error on comunication
        DMCERR << "Error getting device initialization parameter for " << device_network_address->node_id;
        return -101;
    }
    auto_ptr<CDataWrapper> device_init_setting(tmp_cdw_ptr);
        //we can proceed wi the initilization
    auto_ptr<CDataWrapper> initResult(sendRequest(device_network_address->node_id,
                                                  NodeDomainAndActionRPC::ACTION_NODE_INIT,
                                                  device_init_setting.get(),
                                                  millisec_to_wait));
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::deinitDevice(int32_t millisec_to_wait) {
        //int err = ErrorCode::EC_NO_ERROR;
    CDataWrapper message_data;
    message_data.addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, device_network_address->device_id);
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              NodeDomainAndActionRPC::ACTION_NODE_DEINIT,
                                              &message_data,
                                              millisec_to_wait));
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::startDevice(int32_t millisec_to_wait) {
        //int err = ErrorCode::EC_NO_ERROR;
    CDataWrapper message_data;
    message_data.addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, device_network_address->device_id);
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              NodeDomainAndActionRPC::ACTION_NODE_START,
                                              &message_data,
                                              millisec_to_wait));
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::stopDevice(int32_t millisec_to_wait) {
        //int err = ErrorCode::EC_NO_ERROR;
    CDataWrapper message_data;
    message_data.addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, device_network_address->device_id);
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              NodeDomainAndActionRPC::ACTION_NODE_STOP,
                                              &message_data,
                                              millisec_to_wait));
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::restoreDeviceToTag(const std::string& restore_tag, int32_t millisec_to_wait) {
        //int err = ErrorCode::EC_NO_ERROR;
    CDataWrapper message_data;
    message_data.addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, device_network_address->device_id);
    message_data.addStringValue(NodeDomainAndActionRPC::ACTION_NODE_RESTORE_PARAM_TAG, restore_tag);
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              NodeDomainAndActionRPC::ACTION_NODE_RESTORE,
                                              &message_data,
                                              millisec_to_wait));
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::getType(std::string& control_unit_type, int32_t millisec_to_wait) {
        //
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              NodeDomainAndActionRPC::ACTION_CU_GET_INFO,
                                              NULL,
                                              millisec_to_wait));
    if(getLastErrorCode() == ErrorCode::EC_NO_ERROR) {
        if(result.get() && result->hasKey(NodeDefinitionKey::NODE_TYPE)){
            control_unit_type = result->getStringValue(NodeDefinitionKey::NODE_TYPE);
        }
    }
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::getState(CUStateKey::ControlUnitState& deviceState, int32_t millisec_to_wait) {
    CDataWrapper message_data;
    deviceState=CUStateKey::UNDEFINED;
    message_data.addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, device_network_address->device_id);
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              NodeDomainAndActionRPC::ACTION_NODE_GET_STATE,
                                              &message_data,
                                              millisec_to_wait));
    if(getLastErrorCode() == ErrorCode::EC_NO_ERROR) {
        if(result.get() && result->hasKey(CUStateKey::CONTROL_UNIT_STATE)){
            deviceState = (CUStateKey::ControlUnitState)result->getInt32Value(CUStateKey::CONTROL_UNIT_STATE);
        }
    }
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::setAttributeValue(CDataWrapper& attributesValues,
                                            bool noWait,
                                            int32_t millisec_to_wait) {
        //create the pack
    attributesValues.addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, device_network_address->device_id);
    if(noWait){
        sendMessage(device_network_address->node_id, ControlUnitNodeDomainAndActionRPC::CONTROL_UNIT_APPLY_INPUT_DATASET_ATTRIBUTE_CHANGE_SET, &attributesValues);
    } else {
        auto_ptr<CDataWrapper> initResult(sendRequest(device_network_address->node_id,
                                                      ControlUnitNodeDomainAndActionRPC::CONTROL_UNIT_APPLY_INPUT_DATASET_ATTRIBUTE_CHANGE_SET,
                                                      &attributesValues,
                                                      millisec_to_wait));
    }
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
int DeviceMessageChannel::setScheduleDelay(uint64_t scheduledDealy,
                                           int32_t millisec_to_wait) {
    CDataWrapper message_data;
    message_data.addStringValue(NodeDefinitionKey::NODE_UNIQUE_ID, device_network_address->device_id);
    message_data.addInt64Value(ControlUnitDatapackSystemKey::THREAD_SCHEDULE_DELAY, scheduledDealy);
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              NodeDomainAndActionRPC::ACTION_UPDATE_PROPERTY,
                                              &message_data,
                                              millisec_to_wait));
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();

}

    //------------------------------------
void DeviceMessageChannel::sendCustomMessage(const std::string& action_name,
                                             CDataWrapper* const message_data,
                                             bool queued) {
    sendMessage(device_network_address->node_id,
                action_name,
                message_data,
                !queued);
}

    //------------------------------------
int DeviceMessageChannel::sendCustomRequest(const std::string& action_name,
                                            CDataWrapper* const message_data,
                                            CDataWrapper** result_data,
                                            uint32_t millisec_to_wait,
                                            bool async,
                                            bool queued) {
    auto_ptr<CDataWrapper> result(sendRequest(device_network_address->node_id,
                                              action_name,
                                              message_data,
                                              millisec_to_wait,
                                              async,
                                              !queued));
    if(getLastErrorCode() == ErrorCode::EC_NO_ERROR) {
        *result_data = result.release();
    }
    setOnline(!CHAOS_IS_RPC_SERVER_OFFLINE(getLastErrorCode()));
    return getLastErrorCode();
}

    //------------------------------------
std::auto_ptr<MessageRequestFuture>  DeviceMessageChannel::sendCustomRequestWithFuture(const std::string& action_name,
                                                                                       common::data::CDataWrapper *request_data) {
    return sendRequestWithFuture(device_network_address->node_id,
                                 action_name,
                                 request_data);
}

    //! Send a request for receive RPC information
std::auto_ptr<MessageRequestFuture> DeviceMessageChannel::checkRPCInformation() {
    return NodeMessageChannel::checkRPCInformation(device_network_address->node_id);
}

    //! Send a request for an echo test
std::auto_ptr<MessageRequestFuture> DeviceMessageChannel::echoTest(chaos::common::data::CDataWrapper *echo_data) {
    return NodeMessageChannel::echoTest(echo_data);
}