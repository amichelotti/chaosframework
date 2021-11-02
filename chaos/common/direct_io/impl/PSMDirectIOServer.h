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
#ifndef __CHAOSFramework__PSMDirectIOServer__
#define __CHAOSFramework__PSMDirectIOServer__

#include <string>

#include <chaos/common/direct_io/DirectIOServer.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>

#include <chaos/common/message/MessagePSDriver.h>


namespace chaos {
    namespace common {
        namespace direct_io {
            namespace impl {
                
                typedef enum WorkerType {
                    WorkerTypePriority = 1,
                    WorkerTypeService = 2
                } WorkerType;
                
                DECLARE_CLASS_FACTORY(PSMDirectIOServer, DirectIOServer),
                private PSMBaseClass {
                    REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(PSMDirectIOServer)
                    
                    chaos::common::message::consumer_uptr_t cons;
                    chaos::common::message::producer_uptr_t prod;
                    void messageHandler( chaos::common::message::ele_t& data);
                    void messageError( chaos::common::message::ele_t& data);

                    
                    PSMDirectIOServer(std::string alias);
                    ~PSMDirectIOServer();
                public:
                    
                    //! Initialize instance
                    void init(void *init_data);
                    
                    //! Start the implementation
                    void start();
                    
                    //! Stop the implementation
                    void stop();
                    
                    //! Deinit the implementation
                    void deinit();
                };
            }
        }
    }
}


#endif /* defined(__CHAOSFramework__PSMDirectIOServer__) */
