/*
 * Copyright 2012, 18/10/2017 INFN
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
#include <chaos/cu_toolkit/driver_manager/driver/AbstractClientRemoteIODriver.h>

using namespace chaos::cu::driver_manager::driver;

void AbstractClientRemoteIODriver::driverInit(const char *initParameter) throw (chaos::CException) {
    INFO << "Init driver:"<<initParameter;
    CHECK_ASSERTION_THROW_AND_LOG(isDriverParamInJson(), ERR, -1, "Init parameter need to be formated in a json document");
    
    Json::Value root_param_document = getDriverParamJsonRootElement();
    
    Json::Value jv_endpoint_name = root_param_document["endpoint_name"];
    CHECK_ASSERTION_THROW_AND_LOG((jv_endpoint_name.isNull() == false), ERR, -2, "The endpoint name is mandatory");
    
    //! end point identifier & authorization key
    ExternalUnitClientEndpoint::endpoint_identifier = jv_endpoint_name.asString();
    CHECK_ASSERTION_THROW_AND_LOG((ExternalUnitClientEndpoint::endpoint_identifier.size() > 0), ERR, -4, "The endpoint name is empty");
    
    ClientARIODriver::driverInit(initParameter);
    
}
void AbstractClientRemoteIODriver::driverInit(const chaos::common::data::CDataWrapper& init_parameter) throw(chaos::CException) {
    
    CHECK_ASSERTION_THROW_AND_LOG((init_parameter.isEmpty() == false), ERR, -1, "Init parameter need to be formated in a json document");
    CHECK_ASSERTION_THROW_AND_LOG(init_parameter.hasKey("endpoint_name"), ERR, -2, "The endpoint name is mandatory");
    
    //! end point identifier & authorization key
    ExternalUnitClientEndpoint::endpoint_identifier = init_parameter.getStringValue("endpoint_name");
    CHECK_ASSERTION_THROW_AND_LOG((ExternalUnitClientEndpoint::endpoint_identifier.size() > 0), ERR, -4, "The endpoint name is empty");
    
    ClientARIODriver::driverInit(init_parameter);
}
void AbstractClientRemoteIODriver::driverDeinit() throw (chaos::CException) {
    INFO << "Deinit driver";
    CHAOS_NOT_THROW(ClientARIODriver::driverDeinit();)
}
