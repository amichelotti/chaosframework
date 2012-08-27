/*
 *	EventChannel.h
 *	CHAOSFramework
 *	Created by Claudio Bisegni on 27/08/12.
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

#ifndef __CHAOSFramework__EventChannel__
#define __CHAOSFramework__EventChannel__

#include <string>
#include <chaos/common/event/evt_desc/EventDescriptor.h>
#include <chaos/common/action/EventAction.h>
#include <chaos/common/exception/CException.h>

namespace chaos {

    class MessageBroker;
    
    namespace event{

        namespace channel {
            
                //! Managment for send and receive event
            /*!
             The Event Channel permit the forward of a general event descriptor. Every channel
             can be registerd for receive event from other node.
             */
            class EventChannel : public EventAction {
                
                    // make the broker friendly of this class to access private and protected memeber
                friend class chaos::MessageBroker;
                
                    // channel identification ID
                std::string channelID;
                
                    //broker for event forwarding
                MessageBroker *messageBroker;
                
            protected:
                
                EventChannel(MessageBroker *rootBroker);
                
                virtual ~EventChannel();
                
                void init() throw (CException);
                
                void deinit() throw (CException);
                
                int sendRawEvent(EventDescriptor *newEvent);
                
                    //-------------------inherited--------------------
                virtual void handleEvent(event::EventDescriptor *event) = 0;

                
                void activateChannelEventReception(EventType eventType);
                
            public:

                virtual void activateChannelEventReception() = 0;
                
                void deactivateChannelEventReception();
            };
        }
    }
}
#endif /* defined(__CHAOSFramework__EventChannel__) */
