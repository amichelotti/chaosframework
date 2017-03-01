/*
 *	MDSHistoryAgeingManagement.cpp
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

#include "MDSHistoryAgeingManagement.h"

#include <chaos/common/utility/TimingUtil.h>

using namespace chaos::common::utility;
using namespace chaos::metadata_service::cron_job;

MDSHistoryAgeingManagement::MDSHistoryAgeingManagement(chaos::common::data::CDataWrapper *param):
MDSCronJob(param){}

MDSHistoryAgeingManagement::~MDSHistoryAgeingManagement() {}

void MDSHistoryAgeingManagement::start() {
    last_sequence_found = 0;
    log("Start MDSHistoryAgeingManagement");
}

bool MDSHistoryAgeingManagement::execute(const common::cronous_manager::MapKeyVariant& job_parameter) {
    int err = 0;
    bool need_another_step = false;
    std::string control_unit_found = "";
    //in seconds
    uint32_t control_unit_ageing_time = 0;
    uint64_t last_ageing_perform_time = 0;
    
    uint64_t now = TimingUtil::getTimeStamp();
    if((err = getDataAccess<persistence::data_access::ControlUnitDataAccess>()->reserveControlUnitForAgeingManagement(last_sequence_found,
                                                                                                                      control_unit_found,
                                                                                                                      control_unit_ageing_time,
                                                                                                                      last_ageing_perform_time))){
        log(CHAOS_FORMAT("Error %1% reserving control unit for ageing management", %err));
    } else if(control_unit_found.size()){
        uint64_t next_aged_time = (last_ageing_perform_time + (control_unit_ageing_time*1000));
        bool aged =  next_aged_time < now;
        uint64_t remove_until_ts = now - (control_unit_ageing_time*1000);
        log(CHAOS_FORMAT("Processing ageing for control unit %1%", %control_unit_found));
        if(aged) {
            log(CHAOS_FORMAT("Control unit %1% is gone out of ageing time[%2% seconds], we perform agein trigger", %control_unit_found%control_unit_ageing_time));
            
            
            const std::string output_key	= control_unit_found + DataPackPrefixID::OUTPUT_DATASET_POSTFIX;
            const std::string input_key     = control_unit_found + DataPackPrefixID::INPUT_DATASET_POSTFIX;
            const std::string system_key	= control_unit_found + DataPackPrefixID::SYSTEM_DATASET_POSTFIX;
            const std::string custom_key	= control_unit_found + DataPackPrefixID::CUSTOM_DATASET_POSTFIX;
            const std::string health_key    = control_unit_found + NodeHealtDefinitionKey::HEALT_KEY_POSTFIX;
            const std::string alarm_key    = control_unit_found + DataPackPrefixID::DEV_ALARM_DATASET_POSTFIX;
            if((err = getDataAccess<persistence::data_access::ControlUnitDataAccess>()->eraseControlUnitDataBeforeTS(output_key,
                                                                                                                     remove_until_ts))){
                log(CHAOS_FORMAT("Error erasing key %1% for control unit %2% with error %3%", %output_key%control_unit_found%err));
            } else if((err = getDataAccess<persistence::data_access::ControlUnitDataAccess>()->eraseControlUnitDataBeforeTS(input_key,
                                                                                                                            remove_until_ts))){
                log(CHAOS_FORMAT("Error erasing key %1% for control unit %2% with error %3%", %input_key%control_unit_found%err));
            } else if((err = getDataAccess<persistence::data_access::ControlUnitDataAccess>()->eraseControlUnitDataBeforeTS(system_key,
                                                                                                                            remove_until_ts))){
                log(CHAOS_FORMAT("Error erasing key %1% for control unit %2% with error %3%", %system_key%control_unit_found%err));
            } else if((err = getDataAccess<persistence::data_access::ControlUnitDataAccess>()->eraseControlUnitDataBeforeTS(custom_key,
                                                                                                                            remove_until_ts))){
                log(CHAOS_FORMAT("Error erasing key %1% for control unit %2% with error %3%", %custom_key%control_unit_found%err));
            } else if((err = getDataAccess<persistence::data_access::ControlUnitDataAccess>()->eraseControlUnitDataBeforeTS(health_key,
                                                                                                                            remove_until_ts))){
                log(CHAOS_FORMAT("Error erasing key %1% for control unit %2% with error %3%", %health_key%control_unit_found%err));
            } else if((err = getDataAccess<persistence::data_access::ControlUnitDataAccess>()->eraseControlUnitDataBeforeTS(alarm_key,
                                                                                                                            remove_until_ts))){
                log(CHAOS_FORMAT("Error erasing key %1% for control unit %2% with error %3%", %alarm_key%control_unit_found%err));
            } else if((err = getDataAccess<persistence::data_access::LoggingDataAccess>()->eraseLogBeforTS(control_unit_found,
                                                                                                           remove_until_ts))){
                log(CHAOS_FORMAT("Error erasing logging for control unit %2% with error %3%", %alarm_key%control_unit_found%err));
            }
        }
        getDataAccess<persistence::data_access::ControlUnitDataAccess>()->releaseControlUnitForAgeingManagement(control_unit_found, aged);
        
        need_another_step = true;
    } else {}
    return need_another_step;
}

void MDSHistoryAgeingManagement::end() {
    log("End MDSHistoryAgeingManagement");
}
