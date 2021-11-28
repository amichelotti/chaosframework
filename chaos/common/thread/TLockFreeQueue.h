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
#include <chaos/common/chaos_errors.h>
#include <queue>

#include <chaos/common/pqueue/CObjectProcessingPriorityQueue.h>

#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>

namespace chaos {
namespace common {
namespace thread {

template <typename T, int N = 256>
class TLockFreeQueue {
 private:
  //std::queue<T> the_queue;
  boost::lockfree::queue<T, boost::lockfree::fixed_sized<true> > element_queue;
  mutable ChaosMutex                                           the_mutex, mutex_read;
  int                                                            maxsize;
  ChaosConditionVariable                                      the_condition_variable;
  ChaosConditionVariable                                      some_read;
  boost::atomic<uint32_t> size;
 public:
  ~TLockFreeQueue() {
    maxsize=0;
    unblock();
  }
  void unblock() {

    some_read.notify_all();
    the_condition_variable.notify_all();
  }
  boost::lockfree::queue<T, boost::lockfree::fixed_sized<true> >& get() {
    return element_queue;
  }
  TLockFreeQueue()
      : maxsize(N), element_queue(N),size(0){};
  int push(T const& data, int timeout_ms = 0) {
    if (element_queue.push(data)) {
      size++;
      //notify the withing thread
      the_condition_variable.notify_one();
      return size;
    } else {
      ChaosUniqueLock lock(mutex_read);
      if (timeout_ms > 0) {
        /*boost::system_time const tim =
            boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);*/
        if (CHAOS_WAIT(some_read,lock, timeout_ms) == false) {
          return chaos::ErrorCode::EC_GENERIC_TIMEOUT;
        }

      } else {
        some_read.wait(lock);
      }
    }
    if (element_queue.push(data)) {
      //notify the withing thread
      size++;

      the_condition_variable.notify_one();
      return size;
    }
    LERR_ << "Queue Error pushing size:"<<size;

    return -1;
  }
  uint32_t length()const {return size;}
  bool empty() const {
    return element_queue.empty();
  }
  bool pop(T& popped_value) {
    bool ret = element_queue.pop(popped_value);
    if (ret) {
        size--;
      
      some_read.notify_one();
    }
    return ret;
  }
  int wait_and_pop(T& popped_value, int timeout_ms = 0) {
    if (element_queue.empty()) {
      ChaosUniqueLock lock(the_mutex);

      if (timeout_ms > 0) {
       /* boost::system_time const tim =
            boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);*/
        if (CHAOS_WAIT(the_condition_variable,lock, timeout_ms)) {
          if (pop(popped_value)) {
            return size;
          }
        } else {
          // timeout
          return chaos::ErrorCode::EC_GENERIC_TIMEOUT;
        }
      } else {
        the_condition_variable.wait(lock);
      }
    }
    if (pop(popped_value)) {
      return size;
    }
    
    LERR_ << "Queue Error popping size:"<<size;

    return -2;
  }
};
}  // namespace thread
}  // namespace common
}  // namespace chaos
#endif /* defined(__CHAOSFramework__TLockFreeQueue__) */
