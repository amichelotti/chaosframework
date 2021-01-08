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
#include <chaos/cu_toolkit/driver_manager/driver/BaseBypassDriver.h>

using namespace chaos::cu::driver_manager::driver;
#define ADLAPP_ INFO_LOG_1_P(BaseBypassDriver, "BYPASS")
#define ADLDBG_ DBG_LOG_1_P(BaseBypassDriver, "BYPASS")
#define ADLERR_ ERR_LOG_1_P(BaseBypassDriver, "BYPASS")

BaseBypassDriver::BaseBypassDriver(){}
BaseBypassDriver::~BaseBypassDriver(){}

MsgManagmentResultType::MsgManagmentResult BaseBypassDriver::execOpcode(DrvMsgPtr cmd) {
    //do nothing and return as executed
  //  ADLDBG_<<cmd->id<<"] bypass opcode="<<cmd->opcode;
  //  usleep(1000);
    return MsgManagmentResultType::MMR_EXECUTED;
}
