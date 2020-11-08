/*
 * Copyright 2020, 2017 INFN
 *
 * Andrea Michelotti
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

#ifndef __CHAOSFramework__TLockFreeQueue__
#define __CHAOSFramework__TLockFreeQueue__
#include <queue>
#include <chaos/common/chaos_errors.h>

#include <chaos/common/pqueue/CObjectProcessingPriorityQueue.h>

#include <boost/thread.hpp>
#include  <boost/lockfree/queue.hpp> 

namespace chaos {
    namespace common  {
        namespace thread {
            
            
            
            

            template<typename T, int N=256>
            class TLockFreeQueue
            {
            private:
                //std::queue<T> the_queue;
                boost::lockfree::queue< T, boost::lockfree::fixed_sized<true> > element_queue;
                mutable boost::mutex the_mutex,mutex_read;
                boost::atomic<int> size;
                int maxsize;
                boost::condition_variable the_condition_variable;
                boost::condition_variable some_read;

            public:
                ~TLockFreeQueue(){                              
                     some_read.notify_all();
                 the_condition_variable.notify_all();}
                TLockFreeQueue():maxsize(N),element_queue(N){};
                int push(T const& data) {
                    if(element_queue.push(data)){
                        size++;
                    //notify the withing thread
                        the_condition_variable.notify_all();
                        return size;
                    } else {
                        LDBG_<<"Queue FULL";
                        boost::mutex::scoped_lock lock(mutex_read);
                        some_read.wait(lock); 
                    }
                    if(element_queue.push(data)){
                        size++;
                    //notify the withing thread
                        the_condition_variable.notify_all();
                        return size;
                    }
                    LERR_<<"Queue Error pushing";

                    return -1;
                }
                
                bool empty() const {
                    return element_queue.empty();
                }
                bool pop(T& popped_value) {
                    bool ret=element_queue.pop(popped_value);
                    if(ret){
                        size--;
                        some_read.notify_all();

                        }
                    return ret;
                }
                int wait_and_pop(T& popped_value,int timeout_ms=0){
                    int ret=0;

                    if(element_queue.empty()){
                        boost::mutex::scoped_lock lock(the_mutex);

                        if(timeout_ms){
                             boost::system_time const tim =
                         boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
                            if(the_condition_variable.timed_wait(lock, tim)){
                                if(pop(popped_value)){
                                    return size;
                                }
                            } else {
                                // timeout
                                if(pop(popped_value)){
                                    return size;
                                }
                                return chaos::ErrorCode::EC_GENERIC_TIMEOUT;
                            }
                        } else {
                            the_condition_variable.wait(lock);
                            
                        }
                    
                        
                    }
                    if(pop(popped_value)){
                            return size;
                    }
                    LERR_<<"Queue Error pop size:"<<size;
                    return -1;
                }     
                
            };
        }
    }
}
#endif /* defined(__CHAOSFramework__TLockFreeQueue__) */
