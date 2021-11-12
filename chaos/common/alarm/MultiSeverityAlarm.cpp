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

#include <chaos/common/alarm/MultiSeverityAlarm.h>

using namespace chaos::common::alarm;
using namespace chaos::common::state_flag;

#pragma MultiSeverityAlarmHandler
void MultiSeverityAlarmHandler::alarmChanged(const std::string& alarm_tag,
                                             const std::string& alarm_name,
                                             const int8_t alarm_severity){
    alarmChanged(alarm_tag,
                 alarm_name,
                 static_cast<MultiSeverityAlarmLevel>(alarm_severity));
}


#pragma MultiSeverityAlarm
MultiSeverityAlarm::MultiSeverityAlarm(const std::string& alarm_tag,
                                       const std::string& alarm_name,
                                       const std::string& alarm_description,int32_t max_freq):max_freq_log_ms(max_freq),last_log_ms(0),

AlarmDescription(alarm_tag,
                 alarm_name,
                 alarm_description){
    addState(MultiSeverityAlarmLevelClear,
             "clear",
             "Alarm Clear",
             StateFlagServerityRegular);
    addState(MultiSeverityAlarmLevelWarning,
             "Warning",
             "Alarm Warning",
             StateFlagServerityWarning);
    addState(MultiSeverityAlarmLevelHigh,
             "High",
             "Alarm High",
             StateFlagServerityHigh);
    
}

MultiSeverityAlarm::~MultiSeverityAlarm() {}

void MultiSeverityAlarm::setNewSeverity(const MultiSeverityAlarmLevel new_severity) {
    setCurrentLevel(new_severity);
}

const MultiSeverityAlarmLevel MultiSeverityAlarm::getCurrentSeverityLevel() {
    return static_cast<MultiSeverityAlarmLevel>(getCurrentLevel());
}
