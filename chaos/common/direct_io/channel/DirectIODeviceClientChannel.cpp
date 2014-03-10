/*
 *	DirectIODeviceDatasetClientChannel.cpp
 *	!CHOAS
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

#include <chaos/common/utility/endianess.h>
#include <chaos/common/data/cache/FastHash.h>
#include <chaos/common/direct_io/channel/DirectIODeviceClientChannel.h>
#include <chaos/common/direct_io/DirectIOClient.h>

namespace chaos_data = chaos::common::data;
namespace chaos_cache = chaos::common::data::cache;
using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::channel;
using namespace chaos::common::direct_io::channel::opcode_headers;

DirectIODeviceClientChannel::DirectIODeviceClientChannel(std::string alias):DirectIOVirtualClientChannel(alias, DIODataset_Channel_Index, true) {

}

DirectIODeviceClientChannel::~DirectIODeviceClientChannel() {
	
}

void DirectIODeviceClientChannel::setDeviceID(std::string _device_id) {
	//create the has for the device id
	device_hash = chaos_cache::FastHash::hash(_device_id.c_str(), _device_id.size(), 0);
	
	//keep track of the device id
	device_id = _device_id;
}

int64_t DirectIODeviceClientChannel::putDataOutputChannel(bool cache_it, void *buffer, uint32_t buffer_len) {
	DirectIODataPack *data_pack = new DirectIODataPack();
	std::memset(data_pack, 0, sizeof(DirectIODataPack));
	
	DirectIODeviceChannelHeaderPutOpcode *header_data = new DirectIODeviceChannelHeaderPutOpcode();
	
	data_pack->header.dispatcher_header.raw_data = 0;
	//set opcode
	data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::DeviceChannelOpcodePutOutput);
	//set the endpoint that need the receive the pack on the other side
	data_pack->header.dispatcher_header.fields.route_addr = endpoint;
	//set the channel route index within the endpoint
	data_pack->header.dispatcher_header.fields.channel_idx = channel_route_index;
	
	//set header
	header_data->device_hash = TO_LITTE_ENDNS_NUM(uint32_t, device_hash);
	header_data->cache_tag = TO_LITTE_ENDNS_NUM(uint32_t, cache_it);

	DIRECT_IO_SET_CHANNEL_HEADER(data_pack, header_data, sizeof(DirectIODeviceChannelHeaderPutOpcode))
	DIRECT_IO_SET_CHANNEL_DATA(data_pack, buffer, buffer_len)
	return client_instance->sendPriorityData(data_pack);
}

//! Send device serialization with priority
int64_t DirectIODeviceClientChannel::requestLastOutputData(uint16_t p_server_port, uint16_t s_server_port, uint16_t endpoint_idx) {
	DirectIODataPack *data_pack = new DirectIODataPack();
	std::memset(data_pack, 0, sizeof(DirectIODataPack));
	
	DirectIODeviceChannelHeaderGetOpcode *header_data = new DirectIODeviceChannelHeaderGetOpcode();
		
	data_pack->header.dispatcher_header.raw_data = 0;
        //set opcode
	data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::DeviceChannelOpcodeGetLastOutput);
	//set the endpoint that need the receive the pack on the other side
	data_pack->header.dispatcher_header.fields.route_addr = endpoint;
	//set the channel route index within the endpoint
	data_pack->header.dispatcher_header.fields.channel_idx = channel_route_index;
	
    header_data->field.device_hash = TO_LITTE_ENDNS_NUM(uint32_t, device_hash);
    header_data->field.address = TO_LITTE_ENDNS_NUM(uint64_t, ((DirectIOClientConnection*)client_instance)->getI64Ip());
    header_data->field.p_port = TO_LITTE_ENDNS_NUM(uint16_t, p_server_port);
	header_data->field.s_port = TO_LITTE_ENDNS_NUM(uint16_t, s_server_port);
    header_data->field.endpoint = TO_LITTE_ENDNS_NUM(uint16_t, endpoint_idx);
        //set header
    DIRECT_IO_SET_CHANNEL_HEADER(data_pack, header_data, sizeof(DirectIODeviceChannelHeaderGetOpcode))
	return client_instance->sendPriorityData(data_pack);
}