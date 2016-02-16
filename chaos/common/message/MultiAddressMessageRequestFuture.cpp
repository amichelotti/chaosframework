/*
 *	MultiAddressMessageRequestFuture.cpp
 *	!CHAOS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2015 INFN, National Institute of Nuclear Physics
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

#include <chaos/common/message/MultiAddressMessageChannel.h>
#include <chaos/common/message/MultiAddressMessageRequestFuture.h>

#include <boost/format.hpp>

#define ERROR_MESS_DOMAIN_IN_STR(e,m,d)\
boost::str(boost::format("%1%-%2%(%3%)") % e % m % d)

#define MAMRF_INFO INFO_LOG(MultiAddressMessageRequestFuture)
#define MAMRF_DBG DBG_LOG(MultiAddressMessageRequestFuture)
#define MAMRF_ERR ERR_LOG(MultiAddressMessageRequestFuture)

using namespace chaos::common::message;
//!private destructor
MultiAddressMessageRequestFuture::MultiAddressMessageRequestFuture(chaos::common::message::MultiAddressMessageChannel *_parent_mn_message_channel,
                                                                   const std::string& _action_domain,
                                                                   const std::string& _action_name,
                                                                   chaos::common::data::CDataWrapper *_message_pack,
                                                                   int32_t _timeout_in_milliseconds):
timeout_in_milliseconds(_timeout_in_milliseconds),
parent_mn_message_channel(_parent_mn_message_channel),
action_domain(_action_domain),
action_name(_action_name),
message_pack(_message_pack){
    CHAOS_ASSERT(parent_mn_message_channel);
    //send data
    current_future = parent_mn_message_channel->_sendRequestWithFuture(action_domain,
                                                                       action_name,
                                                                       message_pack.get(),
                                                                       last_used_address);
}

//!public destructor
MultiAddressMessageRequestFuture::~MultiAddressMessageRequestFuture() {
    
}

void MultiAddressMessageRequestFuture::setTimeout(int32_t _timeout_in_milliseconds) {
    timeout_in_milliseconds = _timeout_in_milliseconds;
}

void MultiAddressMessageRequestFuture::switchOnOtherServer() {
    //set index offline
    parent_mn_message_channel->setURLAsOffline(last_used_address);
    MAMRF_INFO << "Server " << last_used_address << " put offline";

    //retrasmission of the datapack
    current_future = parent_mn_message_channel->_sendRequestWithFuture(action_domain,
                                                                       action_name,
                                                                       message_pack.get(),
                                                                       last_used_address);
    if(current_future.get()) {
        MAMRF_INFO << "Retransmission on " << last_used_address;
    } else {
        MAMRF_ERR << "No more server for retrasmission, Retry using all offline server for one time";
        //reuse all server
        //parent_mn_message_channel->checkForAliveService();
        //retrasmission of the datapack
        current_future = parent_mn_message_channel->_sendRequestWithFuture(action_domain,
                                                                           action_name,
                                                                           message_pack.get(),
                                                                           last_used_address);
    }
}

//! wait until data is received
bool MultiAddressMessageRequestFuture::wait() {
    CHAOS_ASSERT(parent_mn_message_channel)
    int retry_on_same_server = 0;
    bool working = true;
    //unitle we have valid future and don't have have answer
    while(current_future.get() &&
          working) {
        MAMRF_DBG << "Waiting on server " << last_used_address;
        //! waith for future
        if(current_future->wait(timeout_in_milliseconds)) {
            if(current_future->isRemoteMeaning()) {
                //we have received from remote server somenthing
                working = false;
            } else {
                //we can have submission error
                if(current_future->getError()) {
                    MAMRF_ERR << "Whe have submisison error:" << current_future->getError() <<
                    " message:"<<current_future->getErrorMessage() << " domain:" <<
                    current_future->getErrorDomain();
                    
                    //switch to another server
                    switchOnOtherServer();
                }
            }
        } else{
            if(retry_on_same_server++ < 3) {
                MAMRF_INFO << "Retry to wait on same server";
                continue;
            } else {
                MAMRF_INFO << "Whe have retryied " << retry_on_same_server << " times on "<<last_used_address;
                switchOnOtherServer();
            }
        }
    }
    
    //retry logic
    //parent_mn_message_channel->service_feeder.checkForAliveService();
    
    return working == false;
}

//! try to get the result waiting for a determinate period of time
chaos::common::data::CDataWrapper *MultiAddressMessageRequestFuture::getResult() {
    CHAOS_ASSERT(current_future.get())
    return current_future->getResult();
}


chaos::common::data::CDataWrapper *MultiAddressMessageRequestFuture::detachResult() {
    CHAOS_ASSERT(current_future.get())
    return current_future->detachResult();
}

int MultiAddressMessageRequestFuture::getError() const {
    CHAOS_ASSERT(current_future.get())
    return current_future->getError();
}

const std::string& MultiAddressMessageRequestFuture::getErrorDomain() const {
    CHAOS_ASSERT(current_future.get())
    return current_future->getErrorDomain();
}

const std::string& MultiAddressMessageRequestFuture::getErrorMessage() const {
    CHAOS_ASSERT(current_future.get())
    return current_future->getErrorMessage();
}

chaos::common::data::CDataWrapper *MultiAddressMessageRequestFuture::detachMessageData() {
    return message_pack.release();
}
