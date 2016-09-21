/*
 *	CronJob.h
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 19/09/2016 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFramework__63972CB_F1A7_494B_A797_754F7CAC49AA_CronJob_h
#define __CHAOSFramework__63972CB_F1A7_494B_A797_754F7CAC49AA_CronJob_h

#include <chaos/common/chaos_types.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/data/CDataVariant.h>

namespace chaos {
    namespace common {
        namespace cronous_manager {
            
            //!forward declaration
            class CronousManager;
            
            typedef enum CronJobState {
                CronJobStateWaiting,
                CronJobStateRunning
            }CronJobState;
            
            //!define map for job parameter
            CHAOS_DEFINE_MAP_FOR_TYPE(std::string, chaos::common::data::CDataVariant, MapKeyVariant);

            
            //! abstraction of a job that can be execution to a specifi intervall of time
            class CronJob {
                friend class CronousManager;
                //!keep track of cron job state
                CronJobState run_state;
                MapKeyVariant job_parameter;
                const std::string job_index;
                
                //!is the timestamp for the next job start
                uint64_t next_ts_start;
                uint64_t repeat_delay;

                //!parse a cdatawrpper to create a parameter map
                void parserCDataWrapperForMapParameter(chaos::common::data::CDataWrapper *param);
            protected:
                //signal the starting of the job
                virtual void start() = 0;
                //!execute the job
                /*!
                 \param job_parameter the parametter of the job
                 \return true if job has finisched, false otherwhise
                 */
                virtual bool execute(const MapKeyVariant& job_parameter) = 0;
                
                //!signal the end of the job
                virtual void end() = 0;
                
                void threadEntry();
                
                void log(const std::string& log_message);
            public:
                CronJob(chaos::common::data::CDataWrapper *job_parameter);
                ~CronJob();
            };
        }
    }
}

#endif /* __CHAOSFramework__63972CB_F1A7_494B_A797_754F7CAC49AA_CronJob_h */
