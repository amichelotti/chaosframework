/*
 *	AbstractWANInterface.cpp
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
#include "AbstractWANInterface.h"

using namespace chaos::wan_proxy::wan_interface;

AbstractWANInterface::AbstractWANInterface(const std::string& alias):
NamedService(alias),
handler(NULL){
	
}

AbstractWANInterface::~AbstractWANInterface() {
	
}

// inherited method
void AbstractWANInterface::init(void *init_data) throw(chaos::CException) {
    std::string param_str = (char*)init_data;
    if(!json_reader.parse(param_str, wan_interface_parameter)) {
        throw chaos::CException(-1, "Error reading json parameter", __PRETTY_FUNCTION__);
    }
}

// inherited method
void AbstractWANInterface::deinit() throw(chaos::CException) {
	
}

// inherited method
void AbstractWANInterface::start() throw(chaos::CException) {
	
}

// inherited method
void AbstractWANInterface::stop() throw(chaos::CException) {
	
}

//inherited method
const std::string& AbstractWANInterface::getUrl() {
	return service_url;
}

Json::Value& AbstractWANInterface::getParameter() {
	return wan_interface_parameter;
}

void AbstractWANInterface::setHandler(BaseWANInterfacelHandler *_handler) {
	handler = _handler;
}