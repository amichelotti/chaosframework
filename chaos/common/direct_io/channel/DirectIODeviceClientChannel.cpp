/*
 *	DirectIODeviceDatasetClientChannel.cpp
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

#include <chaos/common/utility/InetUtility.h>
#include <chaos/common/utility/endianess.h>
#include <chaos/common/utility/DataBuffer.h>
#include <chaos/common/direct_io/channel/DirectIODeviceClientChannel.h>
#include <chaos/common/direct_io/DirectIOClient.h>
#include <chaos/common/utility/UUIDUtil.h>
#include <boost/format.hpp>

#include <string.h>

using namespace chaos::common::data;
using namespace chaos::common::data::cache;
using namespace chaos::common::utility;
using namespace chaos::common::direct_io;
using namespace chaos::common::direct_io::channel;
using namespace chaos::common::direct_io::channel::opcode_headers;

//#define PUT_HEADER_LEN  sizeof(DirectIODeviceChannelHeaderPutOpcode)-sizeof(void*)+device_id.size()
#define PUT_HEADER_LEN(x)  (GET_PUT_OPCODE_FIXED_PART_LEN+((uint32_t)x.size()))

#define DECLARE_AND_CALLOC_DATA_PACK(n)\
DirectIODataPack *n = (DirectIODataPack*)calloc(sizeof(DirectIODataPack), 1);

#define DECLARE_AND_CALLOC_HEADER(x,n)\
x *n = (x*)calloc(sizeof(x),1);

#define DIODCCLAPP_ INFO_LOG(DirectIODeviceClientChannel)
#define DIODCCLDBG_ DBG_LOG(DirectIODeviceClientChannel)
#define DIODCCLERR_ ERR_LOG(DirectIODeviceClientChannel)

DEFINE_CLASS_FACTORY(DirectIODeviceClientChannel, DirectIOVirtualClientChannel);

//define the static deallocator class
DirectIODeviceClientChannel::DirectIODeviceClientChannelDeallocator DirectIODeviceClientChannel::STATIC_DirectIODeviceClientChannelDeallocator;


DirectIODeviceClientChannel::DirectIODeviceClientChannel(std::string alias):
DirectIOVirtualClientChannel(alias, DIODataset_Channel_Index),
answer_server_info(){
    //associate the default static allocator
    header_deallocator = &STATIC_DirectIODeviceClientChannelDeallocator;
}

DirectIODeviceClientChannel::~DirectIODeviceClientChannel() {
    
}

void DirectIODeviceClientChannel::setAnswerServerInfo(uint16_t p_server_port, uint16_t s_server_port, uint16_t answer_endpoint) {
    answer_server_info.p_server_port = p_server_port;
    answer_server_info.s_server_port = s_server_port;
    answer_server_info.endpoint = answer_endpoint;
    //setEndpoint(endpoint);
    answer_server_info.ip = ((DirectIOClientConnection*)client_instance)->getI64Ip();
}

int64_t DirectIODeviceClientChannel::storeAndCacheDataOutputChannel(const std::string& key,
                                                                    void *buffer,
                                                                    uint32_t buffer_len,
                                                                    DataServiceNodeDefinitionType::DSStorageType _put_mode,
                                                                    bool wait_result) {
    int64_t err = 0;
    DirectIODataPack *answer = NULL;
    DirectIODataPack *data_pack = (DirectIODataPack*)calloc(sizeof(DirectIODataPack), 1);
    DirectIODeviceChannelHeaderPutOpcode *put_opcode_header = (opcode_headers::DirectIODeviceChannelHeaderPutOpcode *)calloc((PUT_HEADER_LEN(key)+key.size()), 1);
    
    put_opcode_header->tag = (uint8_t) _put_mode;
    put_opcode_header->key_len = key.size();
    //put_opcode_header->key_data
    std::memcpy(GET_PUT_OPCODE_KEY_PTR(put_opcode_header), key.c_str(), put_opcode_header->key_len);
    
    //set opcode
    data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::DeviceChannelOpcodePutOutput);
    
    //set the header
    DIRECT_IO_SET_CHANNEL_HEADER(data_pack, put_opcode_header, (uint32_t)PUT_HEADER_LEN(key))
    //set data if the have some
    if(buffer_len){DIRECT_IO_SET_CHANNEL_DATA(data_pack, buffer, buffer_len);}
    if(wait_result) {
        if((err = sendPriorityData(data_pack, &answer))) {
            //error getting last value
            DIODCCLERR_ << "Error storing value for key:" << key << " with error:" <<err;
        }
    } else {
        err = sendPriorityData(data_pack);
    }
    if(answer) {free(answer);}
    return err;
}

int64_t DirectIODeviceClientChannel::storeAndCacheHealthData(const std::string& key,
                                                             void *buffer,
                                                             uint32_t buffer_len,
                                                             DataServiceNodeDefinitionType::DSStorageType _put_mode,
                                                             bool wait_result) {
    int64_t err = 0;
    DirectIODataPack *answer = NULL;
    DirectIODataPack *data_pack = (DirectIODataPack*)calloc(sizeof(DirectIODataPack), 1);
    DirectIODeviceChannelHeaderPutOpcode *put_opcode_header = (opcode_headers::DirectIODeviceChannelHeaderPutOpcode *)calloc((PUT_HEADER_LEN(key)+key.size()), 1);
    
    put_opcode_header->tag = (uint8_t) _put_mode;
    put_opcode_header->key_len = key.size();
    //put_opcode_header->key_data
    std::memcpy(GET_PUT_OPCODE_KEY_PTR(put_opcode_header), key.c_str(), put_opcode_header->key_len);
    
    //set opcode
    data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::DeviceChannelOpcodePutHeathData);
    
    //set the header
    DIRECT_IO_SET_CHANNEL_HEADER(data_pack, put_opcode_header, (uint32_t)PUT_HEADER_LEN(key))
    //set data if the have some
    if(buffer_len){DIRECT_IO_SET_CHANNEL_DATA(data_pack, buffer, buffer_len);}
    if(wait_result) {
        if((err = sendPriorityData(data_pack, &answer))) {
            //error getting last value
            DIODCCLERR_ << "Error storing value for key:" << key << " with error:" <<err;
        }
    } else {
        err = sendPriorityData(data_pack);
    }
    if(answer) {free(answer);}
    return err;
}

//! Send device serialization with priority
int64_t DirectIODeviceClientChannel::requestLastOutputData(const std::string& key,
                                                           void **result,
                                                           uint32_t &size) {
    int64_t err = 0;
    DirectIODataPack *answer = NULL;
    DirectIODataPack *data_pack = (DirectIODataPack*)calloc(sizeof(DirectIODataPack), 1);
    
    //the precomputed header for get last shared output channel
    DirectIODeviceChannelHeaderGetOpcode *get_opcode_header  = (DirectIODeviceChannelHeaderGetOpcode*)calloc(sizeof(DirectIODeviceChannelHeaderGetOpcode), 1);
    
    //allocate memory for key
    void *data = (void*)malloc(key.size());
    std::memcpy(data, key.c_str(), key.size());
    
    get_opcode_header->field.address = TO_LITTEL_ENDNS_NUM(uint64_t, answer_server_info.ip);
    get_opcode_header->field.p_port = TO_LITTEL_ENDNS_NUM(uint16_t, answer_server_info.p_server_port);
    get_opcode_header->field.s_port = TO_LITTEL_ENDNS_NUM(uint16_t, answer_server_info.s_server_port);
    get_opcode_header->field.endpoint = TO_LITTEL_ENDNS_NUM(uint16_t, answer_server_info.endpoint);
    
    //set opcode
    data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::DeviceChannelOpcodeGetLastOutput);
    
    //set header
    DIRECT_IO_SET_CHANNEL_HEADER(data_pack, get_opcode_header, sizeof(DirectIODeviceChannelHeaderGetOpcode))
    DIRECT_IO_SET_CHANNEL_DATA(data_pack, data, (uint32_t)key.size())
    //send data with synchronous answer flag
    if((err = sendServiceData(data_pack, &answer))) {
        //error getting last value
        DIODCCLERR_ << "Error getting last value for key:" << key << " with error:" <<err;
        *result = NULL;
        if(answer){
            free(answer);
        }
        return err;
    } else {
        //we got answer
        if(answer) {
            //get the header
            opcode_headers::DirectIODeviceChannelHeaderGetOpcodeResult *result_header = static_cast<opcode_headers::DirectIODeviceChannelHeaderGetOpcodeResult*>(answer->channel_header_data);
            result_header->value_len = FROM_LITTLE_ENDNS_NUM(uint32_t, result_header->value_len);
            if(result_header->value_len > 0) {
                size = result_header->value_len;
                *result = answer->channel_data;
            }
        } else {
            *result = NULL;
            size = 0;
        }
    }
    if(answer) {
        if(answer->channel_header_data) free(answer->channel_header_data);
        free(answer);
    }
    return err;
}

int64_t DirectIODeviceClientChannel::requestLastOutputData(const ChaosStringVector& keys,
                                                           VectorCDWShrdPtr& results) {
    if(keys.size() == 0) return -1;
    int64_t err = 0;
    DataBuffer<> data_buffer;
    DirectIODataPack *answer = NULL;
    DirectIODataPack *data_pack = (DirectIODataPack*)calloc(sizeof(DirectIODataPack), 1);
    //set opcode
    data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::DeviceChannelOpcodeMultiGetLastOutput);
    
    DirectIODeviceChannelHeaderMultiGetOpcode *mget_opcode_header  = (DirectIODeviceChannelHeaderMultiGetOpcode*)calloc(sizeof(DirectIODeviceChannelHeaderMultiGetOpcode), 1);
    mget_opcode_header->field.number_of_key = TO_LITTEL_ENDNS_NUM(uint16_t, keys.size());
    
    for(ChaosStringVectorConstIterator it = keys.begin(),
        end = keys.end();
        it != end;
        it++) {
        data_buffer.writeByte(it->c_str(), (int32_t)it->size());
        data_buffer.writeByte('\0');
    }
    
    //set header
    uint32_t data_size = data_buffer.getCursorLocation();
    void* data = data_buffer.release();
    DIRECT_IO_SET_CHANNEL_HEADER(data_pack, mget_opcode_header, sizeof(DirectIODeviceChannelHeaderGetOpcode))
    DIRECT_IO_SET_CHANNEL_DATA(data_pack, data, data_size);
    //send data with synchronous answer flag
    if((err = sendServiceData(data_pack, &answer))) {
        //error getting last value
        DIODCCLERR_ << "Error getting last value for multikey set with error:" << err;
        if(answer){
            free(answer);
        }
        return err;
    } else {
        //we got answer
        if(answer) {
            //get the header
            opcode_headers::DirectIODeviceChannelHeaderMultiGetOpcodeResult *result_header = static_cast<opcode_headers::DirectIODeviceChannelHeaderMultiGetOpcodeResult*>(answer->channel_header_data);
            result_header->number_of_result = FROM_LITTLE_ENDNS_NUM(uint32_t, result_header->number_of_result);
            CHAOS_ASSERT(result_header->number_of_result > 0);
            CHAOS_ASSERT(answer->channel_data);
            
            DataBuffer<> data_buffer(answer->channel_data, answer->header.channel_data_size);
            answer->channel_data = NULL;
            for(int idx = 0;
                idx < result_header->number_of_result;
                idx++) {
                results.push_back(data_buffer.readCDataWrapperAsShrdPtr());
            }
        }
    }
    if(answer) {
        if(answer->channel_header_data) free(answer->channel_header_data);
        if(answer->channel_data) free(answer->channel_data);
        free(answer);
    }
    return err;
}

int64_t DirectIODeviceClientChannel::queryDataCloud(const std::string& key,
                                                    uint64_t start_ts,
                                                    uint64_t end_ts,
                                                    uint32_t page_dimension,
                                                    SearchSequence& last_sequence_id,
                                                    QueryResultPage& found_element_page) {
    int64_t err = 0;
    DirectIODataPack *answer = NULL;
    CDataWrapper query_description;
    //allcoate the data to send direct io pack
    DirectIODataPack *data_pack = (DirectIODataPack*)calloc(sizeof(DirectIODataPack), 1);
    DirectIODeviceChannelHeaderOpcodeQueryDataCloudPtr query_data_cloud_header =
    (DirectIODeviceChannelHeaderOpcodeQueryDataCloud*)calloc(sizeof(DirectIODeviceChannelHeaderOpcodeQueryDataCloud), 1);
    
    //fill the query CDataWrapper
    query_description.addStringValue(DeviceChannelOpcodeQueryDataCloudParam::QUERY_PARAM_SEARCH_KEY_STRING, key);
    query_description.addInt64Value(DeviceChannelOpcodeQueryDataCloudParam::QUERY_PARAM_STAR_TS_I64, (int64_t)start_ts);
    query_description.addInt64Value(DeviceChannelOpcodeQueryDataCloudParam::QUERY_PARAM_END_TS_I64, (int64_t)end_ts);
    query_description.addInt64Value(DeviceChannelOpcodeQueryDataCloudParam::QUERY_PARAM_SEARCH_LAST_RUN_ID, last_sequence_id.run_id);
    query_description.addInt64Value(DeviceChannelOpcodeQueryDataCloudParam::QUERY_PARAM_SEARCH_LAST_DP_COUNTER, last_sequence_id.datapack_counter);
    
    //copy the query id on header
    query_data_cloud_header->field.record_for_page = TO_LITTEL_ENDNS_NUM(uint32_t, page_dimension);
    //set opcode
    data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::DeviceChannelOpcodeQueryDataCloud);
    
    //get the buffer to send
    ChaosUniquePtr<SerializationBuffer> buffer(query_description.getBSONData());
    
    //the frre of memeory is managed in this class in async way
    buffer->disposeOnDelete = false;
    
    //set header and data for the query
    DIRECT_IO_SET_CHANNEL_HEADER(data_pack, query_data_cloud_header, sizeof(DirectIODeviceChannelHeaderOpcodeQueryDataCloud))
    DIRECT_IO_SET_CHANNEL_DATA(data_pack, (void*)buffer->getBufferPtr(), (uint32_t)buffer->getBufferLen());
    if((err = sendServiceData(data_pack, &answer))) {
        //error getting last value
        DIODCCLERR_ << CHAOS_FORMAT("Error executing query for key %1%",%key);
    } else {
        //we got answer
        if(answer) {
            //get the header
            opcode_headers::DirectIODeviceChannelHeaderOpcodeQueryDataCloudResult *result_header = static_cast<opcode_headers::DirectIODeviceChannelHeaderOpcodeQueryDataCloudResult*>(answer->channel_header_data);
            
            uint32_t result_data_size = FROM_LITTLE_ENDNS_NUM(uint32_t, result_header->result_data_size);
            uint32_t numer_of_record_found = FROM_LITTLE_ENDNS_NUM(uint32_t, result_header->numer_of_record_found);
            last_sequence_id.run_id = FROM_LITTLE_ENDNS_NUM(uint64_t, result_header->last_found_sequence.run_id);
            last_sequence_id.datapack_counter = FROM_LITTLE_ENDNS_NUM(uint64_t, result_header->last_found_sequence.datapack_counter);
            if(result_data_size > 0) {
                //scan all result
                char *current_data_prt = (char*)answer->channel_data;
                while(found_element_page.size() < numer_of_record_found) {
                    ChaosSharedPtr<CDataWrapper> last_record(new CDataWrapper(current_data_prt));
                    //!at this time cdata wrapper copy the data
                    found_element_page.push_back(last_record);
                    //clear current record
                    current_data_prt += last_record->getBSONRawSize();
                }
            }
            free(answer->channel_data);
            free(answer->channel_header_data);
        }
        free(answer);
    }
    
    return err;
}

int64_t DirectIODeviceClientChannel::deleteDataCloud(const std::string& key,
                                                     uint64_t start_ts,
                                                     uint64_t end_ts) {
    int64_t err = 0;
    CDataWrapper query_description;
    //allcoate the data to send direct io pack
    DirectIODataPack *data_pack = (DirectIODataPack*)calloc(sizeof(DirectIODataPack), 1);
    
    //fill the query CDataWrapper
    query_description.addStringValue(DeviceChannelOpcodeQueryDataCloudParam::QUERY_PARAM_SEARCH_KEY_STRING, key);
    query_description.addInt64Value(DeviceChannelOpcodeQueryDataCloudParam::QUERY_PARAM_STAR_TS_I64, (int64_t)start_ts);
    query_description.addInt64Value(DeviceChannelOpcodeQueryDataCloudParam::QUERY_PARAM_END_TS_I64, (int64_t)end_ts);
    
    //set opcode
    data_pack->header.dispatcher_header.fields.channel_opcode = static_cast<uint8_t>(opcode::DeviceChannelOpcodeDeleteDataCloud);
    
    //get the buffer to send
    ChaosUniquePtr<SerializationBuffer> buffer(query_description.getBSONData());
    
    //the frre of memeory is managed in this class in async way
    buffer->disposeOnDelete = false;
    
    //set header and data for the query
    DIRECT_IO_SET_CHANNEL_DATA(data_pack, (void*)buffer->getBufferPtr(), (uint32_t)buffer->getBufferLen());
    if((err = sendServiceData(data_pack))) {
        //error getting last value
        DIODCCLERR_ << CHAOS_FORMAT("Error executing deelte operation for key %1%",%key);
    }
    
    return err;
}

//! default data deallocator implementation
void DirectIODeviceClientChannel::DirectIODeviceClientChannelDeallocator::freeSentData(void* sent_data_ptr, DisposeSentMemoryInfo *free_info_ptr) {
    switch(free_info_ptr->sent_part) {
        case DisposeSentMemoryInfo::SentPartHeader:{
            switch(static_cast<opcode::DeviceChannelOpcode>(free_info_ptr->sent_opcode)) {
                case opcode::DeviceChannelOpcodePutOutput:
                case opcode::DeviceChannelOpcodePutHeathData:
                case opcode::DeviceChannelOpcodeGetLastOutput:
                case opcode::DeviceChannelOpcodeQueryDataCloud:
                case opcode::DeviceChannelOpcodeMultiGetLastOutput:
                    free(sent_data_ptr);
                    break;
                    //these two opcode are header allocated in the stack
                case opcode::DeviceChannelOpcodeDeleteDataCloud:
                    break;
            }
            break;
        }
            
        case DisposeSentMemoryInfo::SentPartData: {
            switch(static_cast<opcode::DeviceChannelOpcode>(free_info_ptr->sent_opcode)) {
                    //opcode with data
                case opcode::DeviceChannelOpcodePutOutput:
                case opcode::DeviceChannelOpcodePutHeathData:
                case opcode::DeviceChannelOpcodeGetLastOutput:
                case opcode::DeviceChannelOpcodeQueryDataCloud:
                case opcode::DeviceChannelOpcodeDeleteDataCloud:
                case opcode::DeviceChannelOpcodeMultiGetLastOutput:
                    free(sent_data_ptr);
                    break;
            }
            break;
        }
    }
}
