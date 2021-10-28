//
//  PSMClient.h
//  CHAOSFramework
//
//  Created by Bisegni Claudio on 11/03/12.
//  Copyright (c) 2012 INFN. All rights reserved.
//

#ifndef CHAOSFramework_PSMClient_h
#define CHAOSFramework_PSMClient_h

//#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#include <chaos/common/rpc/RpcClient.h>
#include <chaos/common/pqueue/ChaosProcessingQueue.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>
#include <chaos/common/utility/TimingUtil.h>
#include <chaos/common/pool/ResourcePool.h>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <chaos/common/message/MessagePSDriver.h>
#include <map>
#include <deque>
namespace chaos {
    
    class PSMClient;
    class SocketEndpointPool;
    
    struct PSMSocketPoolDef{
        void * socket;
        std::string identity;
    };
    
    typedef chaos::common::pool::ResourcePool<PSMSocketPoolDef> PSMSocketPool;
    
    //define the pool my for every endpoint
    CHAOS_DEFINE_MAP_FOR_TYPE(std::string, ChaosSharedPtr< PSMSocketPool >, SocketMap)
    
    /*
     Class that implemnt !CHAOS RPC messaggin gusing PSM
     
     driver parameter:
     key:zmq_timeout value is a stirng that represent the integer used as timeout
     */
    DECLARE_CLASS_FACTORY(PSMClient, RpcClient),
    public chaos::common::async_central::TimerHandler {
        REGISTER_AND_DEFINE_DERIVED_CLASS_FACTORY_HELPER(PSMClient)
        PSMClient(const std::string& alias);
        virtual ~PSMClient();
        boost::shared_mutex map_socket_mutex;
        ChaosAtomic<uint64_t> seq_id;
        std::string nodeuid;
    protected:        
        //timer handler
        void timeout();
        
        // publish subscribe
		chaos::common::message::producer_uptr_t prod;
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
        
        /*
         Submit the message to be send to a certain ip, the datawrapper must contains
         the key CS_CMDM_REMOTE_HOST_IP
         */
        bool submitMessage(NFISharedPtr forwardInfo, bool onThisThread=false);
        
        //inherited method
        virtual uint64_t getMessageQueueSize();
    };
}
#endif
