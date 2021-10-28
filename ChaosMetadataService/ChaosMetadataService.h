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

#ifndef ChaosMetadataService_H
#define ChaosMetadataService_H
//#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#include "mds_constants.h"
#include "mds_types.h"
#include "api/ApiManagment.h"
#include "cron_job/MDSCronusManager.h"
#include "QueryDataConsumer.h"

#include <boost/thread/condition.hpp>

#include <chaos/common/global.h>
#include <chaos/common/ChaosCommon.h>
#include <chaos/common/async_central/async_central.h>
#include <chaos/common/network/NetworkBroker.h>
#include <chaos/common/thread/WaitSemaphore.h>
#include <chaos/common/utility/StartableService.h>
#include <chaos/common/utility/ProcStat.h>
namespace chaos {
    namespace metadata_service {
        //! Chaos Node Directory base class
        class ChaosMetadataService :
        public ChaosCommon<ChaosMetadataService>,
        public chaos::common::async_central::TimerHandler,
        public ServerDelegator {
            friend class chaos::common::utility::Singleton<ChaosMetadataService>;
             
            static WaitSemaphore waitCloseSemaphore;
            //!persistence driver instance
            chaos::common::utility::InizializableServiceContainer<api::ApiManagment> api_managment_service;
            //! CDS data consumer that respond to data api
            chaos::common::utility::StartableServiceContainer<QueryDataConsumer> data_consumer,message_consumer;
            
            //keep track of process resource usage
            //ProcStat service_proc_stat;
            
            ChaosMetadataService();
            ~ChaosMetadataService();
            static void signalHanlder(int);
            
            //! convert param_key to a string of string hash map
            void fillKVParameter(std::map<std::string, std::string>& kvmap,
                                 const std::vector<std::string>& multitoken_param);
            //inherited by chaos::common::async_central::TimerHandler
            void timeout();
            bool is_present;// check if exists the entry
        public:
            static uint64_t timePrecisionMask;
            static  std::string mdsName;

            struct setting	setting;
            uint32_t timeError_opt;
            typedef boost::mutex::scoped_lock lock;
            //! C and C++ attribute parser
            /*!
             Specialized option for startup c and cpp program main options parameter
             */
            void init(int argc, const char* argv[]);
            //!stringbuffer parser
            /*
             specialized option for string stream buffer with boost semantics
             */
            void init(istringstream &initStringStream);
            void init(void *init_data);
            void start();
            void stop();
            void deinit();
            /**
             * @brief Look on cache to retrive live nodes
             * 
             * @param uids vector of nodes
             * @return std::vector<bool> return a corresponding array of bool
             */
            std::vector<bool> areNodeAlive(const ChaosStringVector& uids);
            bool isNodeAlive(const std::string& uid);
            std::map <std::string,int64_t> alive_cache;
            /**
             * @brief update alive_cache if dataset timestamp is newwer
             * 
             */
            void updateLiveCache(const chaos::common::data::CDataWrapper*);

            /**
             * @brief update alive_cache if ts is newer
             * 
             */
            void updateLiveCache(const std::string& name,int64_t te);


            /**
             * @brief remove storage data to from
             * 
             * 
             */
            int removeStorageData(const std::string& nodeuid,uint64_t start,uint64_t end);

            /**
             * @brief used to notify a new registration on the data consumer logic( i.e. on registration the cds subscribe to it)
             * 
             * @param nodeuid 
             * @return int 
             */
            int notifyNewNode(const std::string& nodeuid);
        };
    }
}
//#pragma GCC visibility pop
#endif
