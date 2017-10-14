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

#ifndef __CHAOSFramework__8328273_33D4_43F5_8FD4_B0B6493427DC_RemoteIODriver_h
#define __CHAOSFramework__8328273_33D4_43F5_8FD4_B0B6493427DC_RemoteIODriver_h

#include <chaos/cu_toolkit/driver_manager/driver/AbstractRemoteIODriver.h>
#include <chaos/cu_toolkit/driver_manager/driver/OpcodeExternalCommandMapper.h>
#include <chaos/cu_toolkit/driver_manager/driver/RemoteIODriverProtocol.h>
namespace chaos {
    namespace cu {
        namespace driver_manager {
            namespace driver {
                //!permit to specify the converter to adapt already created driver for be used as external
                template<typename OpExCMDAdaptor, typename ExtDriverImpl>
                class OpcodeDriverWrapper:
                public ExtDriverImpl,
                public RemoteIODriverProtocol {
                    ChaosUniquePtr<OpcodeExternalCommandMapper> opcode_ext_cmd_mapper;
                    virtual MsgManagmentResultType::MsgManagmentResult execOpcode(DrvMsgPtr cmd) {
                        return opcode_ext_cmd_mapper->execOpcode(cmd);
                    }
                    
                    int asyncMessageReceived(chaos::common::data::CDWUniquePtr message) {
                        return opcode_ext_cmd_mapper->asyncMessageReceived(ChaosMoveOperator(message));
                    }
                    
                    
                public:
                    OpcodeDriverWrapper():
                    opcode_ext_cmd_mapper(new OpExCMDAdaptor(this)){}
                    ~OpcodeDriverWrapper() {}
                    
                    //!Send raw request to the remote driver
                    /*!
                     \param message_data is the raw data to be transmitted to the remote driver
                     \param received_data si the raw data received from the driver
                     */
                    int sendRawRequest(chaos::common::data::CDWUniquePtr message_data,
                                       chaos::common::data::CDWShrdPtr& message_response,
                                       uint32_t timeout = 5000) {
                        return ExtDriverImpl::sendRawRequest(ChaosMoveOperator(message_data),
                                                           message_response,
                                                           timeout);
                    }
                    
                    //!Send raw message to the remote driver
                    /*!
                     \param message_data is the raw data to be transmitted to the remote driver
                     */
                    int sendRawMessage(chaos::common::data::CDWUniquePtr message_data) {
                        return ExtDriverImpl::sendRawMessage(ChaosMoveOperator(message_data));
                    }
                };
            }
        }
    }
}

#endif /* __CHAOSFramework__8328273_33D4_43F5_8FD4_B0B6493427DC_RemoteIODriver_h */