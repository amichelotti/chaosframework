/*
 *	SlowCommand.cpp
 *	!CHAOS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2012 INFN, National Institute of Nuclear Physics
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
#include <string>
#include <chaos/cu_toolkit/control_manager/slow_command/SlowCommand.h>
using namespace chaos;
using namespace chaos::common::data;
using namespace chaos::cu::control_manager;
using namespace chaos::cu::control_manager::slow_command;
#define SCLOG_HEAD_SL "[SlowCommand-" << device_id << "-" << unique_id << "] "
#define SCLAPP_ LAPP_ << SCLOG_HEAD_SL
#define SCLDBG_ LDBG_ << SCLOG_HEAD_SL
#define SCLERR_ LERR_ << SCLOG_HEAD_SL

// default constructor
SlowCommand::SlowCommand() {
    
    
}

// default destructor
SlowCommand::~SlowCommand() {
    
}

const string & SlowCommand::getDeviceID() {
    return abstract_control_unit->getDeviceID();
}

/*
 return the device database with the dafualt device information
 */
chaos::common::data::DatasetDB * const SlowCommand::getDeviceDatabase() {
    return abstract_control_unit;
}

AttributeSharedCacheWrapper * const SlowCommand::getAttributeCache() {
    return attribute_cache;
}

//!update the system status flag
void SlowCommand::setSystemStatusFlag(StatusFlagType flag_type,
                                      bool new_flag_value) {
    abstract_control_unit->setSystemStatusFlag(flag_type,
                                               new_flag_value);
}

//! return the value of a specific system flag
const bool SlowCommand::getSystemStatsuFlag(StatusFlagType flag_type) {
    return abstract_control_unit->getSystemStatsuFlag(flag_type);
}

//! update status flag and push
void SlowCommand::updateAndPusblishStatusFlag(StatusFlagType flag_type,
                                              bool new_flag_value) {
    abstract_control_unit->updateAndPusblishStatusFlag(flag_type,
                                                       new_flag_value);
}
