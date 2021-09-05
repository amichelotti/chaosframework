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

#include "DriverPropertyCU.h"

using namespace chaos;
using namespace chaos::common::data::cache;
using namespace chaos::cu::driver_manager::driver;

PUBLISHABLE_CONTROL_UNIT_IMPLEMENTATION(chaos::cu::control_manager::DriverPropertyCU)

#define DriverPropertyCULAPP_ LAPP_ << "[DriverPropertyCU] "
#define DriverPropertyCULDBG_ LDBG_ << "[DriverPropertyCU] " << __PRETTY_FUNCTION__ << " "
#define DriverPropertyCULERR_ LERR_ << "[DriverPropertyCU] " << __PRETTY_FUNCTION__ << "(" << __LINE__ << ") "

using namespace chaos::cu::control_manager;
/*
 Construct
 */
DriverPropertyCU::DriverPropertyCU(const string& _control_unit_id, const string& _control_unit_param, const ControlUnitDriverList& _control_unit_drivers)
    : RTAbstractControlUnit(_control_unit_id, _control_unit_param, _control_unit_drivers),driver(NULL) {

}

/*
 Destructor
 */
DriverPropertyCU::~DriverPropertyCU() {
}

//!Return the definition of the control unit
/*!
 The api that can be called withi this method are listed into
 "Control Unit Definition Public API" module into html documentation
 (chaosframework/Documentation/html/group___control___unit___definition___api.html)
 */
void DriverPropertyCU::unitDefineActionAndDataset() {
  DriverPropertyCULAPP_ << "UnitDefine";
  addPublicDriverPropertyToDataset();
  
}

//!Define custom control unit attribute
void DriverPropertyCU::unitDefineCustomAttribute() {
}

//!Initialize the Custom Control Unit
void DriverPropertyCU::unitInit() {
  //driver->init();
}

//!Execute the work, this is called with a determinated delay, it must be as fast as possible
void DriverPropertyCU::unitStart() {
}

//!Execute the Control Unit work
void DriverPropertyCU::unitRun() {
  //get the output attribute pointer form the internal cache
  updateDatasetFromDriverProperty();
 // getAttributeCache()->setOutputDomainAsChanged();
}

//!Execute the Control Unit work
void DriverPropertyCU::unitStop() {
}

//!Deinit the Control Unit
void DriverPropertyCU::unitDeinit() {
 // driver->deinit();
}


/*
 CDataWrapper *DriverPropertyCU::my_custom_action(CDataWrapper *actionParam, bool& detachParam) {
	CDataWrapper *result =  new CDataWrapper();
	return result;
 }
 */
