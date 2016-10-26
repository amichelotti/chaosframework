/*
 *	MultiSeverityAlarm.cpp
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 26/10/2016 INFN, National Institute of Nuclear Physics
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

#include <chaos/common/alarm/MultiSeverityAlarm.h>

using namespace chaos::common::alarm;
using namespace chaos::common::status_manager;

MultiSeverityAlarm::MultiSeverityAlarm(const std::string alarm_name,
                                       const std::string alarm_description):
AlarmDescription(alarm_name,
                 alarm_description){
    addState(MultiSeverityAlarmLevelClear,
             "clear",
             "Alarm is in a regular state",
             StatusFlagServerityRegular);
    addState(MultiSeverityAlarmLevelLow,
             "low",
             "Low probability that something will fails, attention is needed",
             StatusFlagServerityLow);
    addState(MultiSeverityAlarmLevelHig,
             "Severe",
             "High probability that something is going to fails",
             StatusFlagServeritySevere);
    
}

MultiSeverityAlarm::~MultiSeverityAlarm() {
    
}

void MultiSeverityAlarm::setNewSeverity(const MultiSeverityAlarmLevel new_severity) {
    setCurrentLevel(new_severity);
}

const MultiSeverityAlarmLevel MultiSeverityAlarm::getCurrentSeverityLevel() {
    return static_cast<MultiSeverityAlarmLevel>(getCurrentLevel());
}
