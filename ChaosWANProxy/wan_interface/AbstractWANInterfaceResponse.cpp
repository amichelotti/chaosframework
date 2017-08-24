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
#include "AbstractWANInterfaceResponse.h"

using namespace chaos::wan_proxy::wan_interface;

AbstractWANInterfaceResponse::AbstractWANInterfaceResponse():
code(HTTP_OK) {
}

AbstractWANInterfaceResponse::AbstractWANInterfaceResponse(const std::string& content_type):
code(HTTP_OK) {
		headers.insert(make_pair("Content-Type", content_type));
}

AbstractWANInterfaceResponse::~AbstractWANInterfaceResponse() {
}


std::map<std::string, std::string>& AbstractWANInterfaceResponse::getHeader() {
	return headers;
}

void AbstractWANInterfaceResponse::addHeaderKeyValue(const std::string& key,
													 const std::string& value) {
	headers.insert(make_pair(key, value));
}

void AbstractWANInterfaceResponse::setCode(int _code) {
	code = _code;
}

int AbstractWANInterfaceResponse::getCode() {
	return code;
}