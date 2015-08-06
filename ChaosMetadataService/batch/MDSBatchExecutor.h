/*
 *	BatchExecutor.h
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

#ifndef __CHAOSFramework__BatchExecutor__
#define __CHAOSFramework__BatchExecutor__

#include "MDSBatchCommand.h"
//#include "../ChaosMetadataService.h"

#include <chaos/common/network/NetworkBroker.h>
#include <chaos/common/batch_command/BatchCommand.h>
#include <chaos/common/batch_command/BatchCommandExecutor.h>

#include <chaos_service_common/persistence/data_access/AbstractPersistenceDriver.h>
namespace chaos{
    namespace metadata_service {
        class ChaosMetadataService;
        
        namespace batch {
            
            /*!
             this class sis the implementation of the batch executor for
             the metadataserver batch job
             */
            class MDSBatchExecutor:
            public chaos::common::batch_command::BatchCommandExecutor {
                friend class chaos::metadata_service::ChaosMetadataService;
                
                chaos::common::network::NetworkBroker *network_broker;
                chaos::common::message::MessageChannel *message_channel_for_job;
                chaos::common::message::MultiAddressMessageChannel *multiaddress_message_channel_for_job;
                
                //dataaccess abstract driver
                chaos::service_common::persistence::data_access::AbstractPersistenceDriver *abstract_persistance_driver;
            protected:
                //allocate a new command
                common::batch_command::BatchCommand *instanceCommandInfo(const std::string& command_alias,
                                                                         chaos::common::data::CDataWrapper *command_info);
                
                //overlodaed command event handler
                void handleCommandEvent(uint64_t command_seq,
                                        common::batch_command::BatchCommandEventType::BatchCommandEventType type,
                                        void* type_value_ptr,
                                        uint32_t type_value_size);
                
                //! general sandbox event handler
                void handleSandboxEvent(const std::string& sandbox_id,
                                        common::batch_command::BatchSandboxEventType::BatchSandboxEventType type,
                                        void* type_value_ptr,
                                        uint32_t type_value_size);
                
            public:
                MDSBatchExecutor(const std::string& executor_id,
                                 chaos::common::network::NetworkBroker *_network_broker);
                ~MDSBatchExecutor();
                
                // Initialize instance
                void init(void *init_data) throw(chaos::CException);
                
                // start instance
                void start() throw(chaos::CException);
                
                // stop instance
                void stop() throw(chaos::CException);
                
                // Deinitialize instance
                void deinit() throw(chaos::CException);
                
                //! Install a command associated with a type
                void installCommand(const std::string& alias,
                                    chaos::common::utility::NestedObjectInstancer<MDSBatchCommand, common::batch_command::BatchCommand> *instancer);
            };
        }
    }
}

#endif /* defined(__CHAOSFramework__BatchExecutor__) */
