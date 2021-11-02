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

#ifndef CHAOSFramework_PSMServer_h
#define CHAOSFramework_PSMServer_h
//#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#include <vector>
#include <boost/thread.hpp>
#include <chaos/common/message/MessagePSDriver.h>

#include <chaos/common/rpc/RpcServer.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>


namespace chaos {
    /*
     Class that implement the Chaos RPC adapter for 0mq protocoll
     */
    DECLARE_CLASS_FACTORY(PSMServer, RpcServer)  {
        REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(PSMServer)
        // publish subscribe
		chaos::common::message::consumer_uptr_t cons;
        chaos::common::message::producer_uptr_t prod;

        std::string nodeuid;
        PSMServer(const std::string& alias);
        virtual ~PSMServer();
        //worker that process request in a separate thread
        void messageHandler( chaos::common::message::ele_t& data);
        void messageError( chaos::common::message::ele_t& data);

    public:
        
        /*
         init the rpc adapter
         */
        void init(void *init_data);
        /*
         start the rpc adapter
         */
        void start();
        /*
         start the rpc adapter
         */
        void stop();
        /*
         deinit the rpc adapter
         */
        void deinit();
        
        //server worker thread
        /*!
         Thread where data is received and managed
         */
        std::string getPublishedEndpoint();
            };
    
}
#endif
