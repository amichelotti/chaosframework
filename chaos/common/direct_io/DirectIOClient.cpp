/*
 *	DirectIOClient.cpp
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
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/utility/InetUtility.h>
#include <chaos/common/direct_io/DirectIOClient.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/direct_io/channel/DirectIOVirtualClientChannel.h>

#include <boost/lexical_cast.hpp>

using namespace chaos::common::direct_io;
namespace chaos_data = chaos::common::data;

namespace b_algo = boost::algorithm;

#define DIO_LOG_HEAD "["<<getName()<<"] - "

#define DIOLAPP_ LAPP_ << DIO_LOG_HEAD
#define DIOLDBG_ LDBG_ << DIO_LOG_HEAD
#define DIOLERR_ LERR_ << DIO_LOG_HEAD

// current client ip in string form
std::string DirectIOClient::my_str_ip;

// current client ip in 64 bit form
uint64_t DirectIOClient::my_i64_ip = 0;

DirectIOClient::DirectIOClient(string alias):NamedService(alias), connection_mode(DirectIOConnectionSpreadType::DirectIONoSetting) {
	
}

DirectIOClient::~DirectIOClient() {
    clearChannelInstancerAndInstance();
}

std::string DirectIOClient::getStrIp() {
    return my_str_ip;
}
uint64_t DirectIOClient::getI64Ip() {
    return my_i64_ip;
}

void DirectIOClient::clearChannelInstancerAndInstance() {
    for(ChannelMapIterator iter = channel_map.begin();
        iter != channel_map.end();
        iter++) {
        delete(iter->second);
    }
    channel_map.clear();
}

// allocate a new channel by the instancer
channel::DirectIOVirtualClientChannel *DirectIOClient::registerChannelInstance(channel::DirectIOVirtualClientChannel *channel_instance) {
    boost::unique_lock<boost::shared_mutex>	Lock(mutex_channel_map);
    if(channel_instance == NULL) return NULL;
    
    //associate the instance
    channel_instance->client_instance = this;
    channel_map.insert(make_pair(channel_instance->channel_route_index, channel_instance));
    return channel_instance;
}

// dispose the channel instance
void DirectIOClient::deregisterChannelInstance(channel::DirectIOVirtualClientChannel *channel_instance) {
    if(channel_instance == NULL) return;
    boost::unique_lock<boost::shared_mutex>	Lock(mutex_channel_map);
    channel_map.erase(channel_instance->channel_route_index);
    delete channel_instance;
}

//! Initialize instance
void DirectIOClient::setConnectionMode(DirectIOConnectionSpreadType::DirectIOConnectionSpreadType _connection_mode) {
    connection_mode = _connection_mode;
}

// New channel allocation by name
channel::DirectIOVirtualClientChannel *DirectIOClient::getNewChannelInstance(std::string channel_name) throw (CException) {
	channel::DirectIOVirtualClientChannel *channel = chaos::ObjectFactoryRegister<channel::DirectIOVirtualClientChannel>::getInstance()->getNewInstanceByName(channel_name.c_str());
	registerChannelInstance(channel);
	return channel;
}

// New channel allocation by name
void DirectIOClient::releaseChannelInstance(channel::DirectIOVirtualClientChannel *channel_instance) throw (CException) {
	deregisterChannelInstance(channel_instance);
	//if(channel_instance) delete(channel_instance);
}
