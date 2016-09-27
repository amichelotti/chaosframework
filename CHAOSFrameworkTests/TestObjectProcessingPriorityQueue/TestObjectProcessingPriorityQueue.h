/*
 *	TestObjectProcessingPriorityQueue.h
 *
 *	!CHAOS [CHAOSFrameworkTests]
 *	Created by bisegni.
 *
 *    	Copyright 22/08/16 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFrameworkTests__E2A0E5A_E2D9_47FC_880A_39C7695501E3_TestObjectProcessingPriorityQueue_h
#define __CHAOSFrameworkTests__E2A0E5A_E2D9_47FC_880A_39C7695501E3_TestObjectProcessingPriorityQueue_h

#include <chaos/common/pqueue/CObjectProcessingPriorityQueue.h>

#include <boost/atomic.hpp>
#include <boost/thread.hpp>

namespace chaos {
    namespace common {
        namespace pqueue {
            namespace test {
                
                typedef struct TestJobPriorityStruct {
                    int priority;
                    boost::atomic<int> check_concurence;
                } TestJobPriorityStruct;
                
                //! thest for the chaos::common::pqueue::CObjectProcessingQueue
                class TestObjectProcessingPriorityQueue:
                protected  CObjectProcessingPriorityQueue<TestJobPriorityStruct> {
                    int number_of_consumer;
                    int number_of_producer;
                    int number_of_production;
                    boost::atomic<int> job_to_do;
                    boost::atomic<int> error_in_job;
                    boost::atomic<int> priority_counter;
                    boost::thread_group tg;
                    void processBufferElement(TestJobPriorityStruct *job,
                                              ElementManagingPolicy& p) throw(CException);
                    
                    void producer();
                public:
                    TestObjectProcessingPriorityQueue();
                    virtual ~TestObjectProcessingPriorityQueue();
                    bool test(int _number_of_producer,
                              int _number_of_production,
                              int _number_of_consumer,
                              unsigned int _sec_to_whait = 0,
                              bool _whait_to_finish = true);
                };
            }
        }
    }
}

#endif /* __CHAOSFrameworkTests__E2A0E5A_E2D9_47FC_880A_39C7695501E3_TestObjectProcessingPriorityQueue_h */