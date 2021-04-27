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

#include <chaos/common/direct_io/DirectIOURLManagment.h>
#if __cplusplus >= 201103L
#include <regex>
using namespace std;
#else
#include <boost/regex.hpp>
using namespace boost;
#endif
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace chaos::common::direct_io;


//! Regular expression for check server endpoint with the sintax hostname:[priority_port:service_port]
#define DirectIOHostIPAndEndpoint  "[a-zA-Z0-9]+(.[a-zA-Z0-9]+)+:[0-9]{4,5}:[0-9]{4,5}\\|[0-9]{1,3}"
//! Regular expression for check server endpoint with the sintax ip:[priority_port:service_port]
#define DirectIOServerDescriptionHostAndEndpoint  "\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b:[0-9]{4,5}:[0-9]{4,5}\\|[0-9]{1,3}"
//! Regular expression for check server endpoint with the sintax hostname:[priority_port:service_port]
#define  DirectIOHostName  "[a-zA-Z0-9]+(.[a-zA-Z0-9]+)+:[0-9]{4,5}:[0-9]{4,5}"
//! Regular expression for check server endpoint with the sintax ip:[priority_port:service_port]
#define DirectIOIPAndPort  "\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b:[0-9]{4,5}:[0-9]{4,5}"
namespace chaos{
	namespace common{
		namespace direct_io{

bool checkURL(const std::string& url){
	// boost regex get invalid free on chaos
	const char _DirectIOHostIPAndEndpoint[]=DirectIOHostIPAndEndpoint;
	const char _DirectIOServerDescriptionHostAndEndpoint[]= DirectIOServerDescriptionHostAndEndpoint ;

	regex DirectIOHostIPAndEndpointRegExp(_DirectIOHostIPAndEndpoint);
    regex DirectIOServerDescriptionHostAndEndpointRegExp(_DirectIOServerDescriptionHostAndEndpoint);
	//boost::smatch match0,match1;
	return	regex_match(url,DirectIOHostIPAndEndpointRegExp) || regex_match(url,DirectIOServerDescriptionHostAndEndpointRegExp);
	
/*
boost::regex DirectIOHostIPAndEndpointRegExp(_DirectIOHostIPAndEndpoint);
    boost::regex DirectIOServerDescriptionHostAndEndpointRegExp(_DirectIOServerDescriptionHostAndEndpoint);
	//boost::smatch match0,match1;
	return	boost::regex_match(url,DirectIOHostIPAndEndpointRegExp) ||
	boost::regex_match(url,DirectIOServerDescriptionHostAndEndpointRegExp);
	*/
return true;
}
/*
bool DirectIOURLManagment::checkURL(const std::string& url) {
return checkUrl();
    
}*/


bool DirectIOURLManagment::decoupleServerDescription(const std::string& server_desc,
													 std::string& priority_desc,
													 std::string& service_desc) {
	std::vector<std::string> server_desc_tokens;
	
    regex DirectIOHostNameRegExp(DirectIOHostName);
    regex DirectIOIPAndPortRegExp(DirectIOIPAndPort);
	if(!regex_match(server_desc, DirectIOHostNameRegExp) &&
       !regex_match(server_desc, DirectIOIPAndPortRegExp)) {
        return false;
    }

	boost::algorithm::split(server_desc_tokens, server_desc, boost::algorithm::is_any_of(":"), boost::algorithm::token_compress_on);
	
	//create the two servers description
	priority_desc = boost::str( boost::format("%1%:%2%") % server_desc_tokens[0] % server_desc_tokens[1]);
	service_desc = boost::str( boost::format("%1%:%2%") % server_desc_tokens[0] % server_desc_tokens[2]);
	return true;
}

bool DirectIOURLManagment::decoupleServerDescription(const std::string& server_desc,
													 std::vector<std::string>& servers_desc) {
	std::string p_desc;
	std::string s_desc;
	if(! decoupleServerDescription(server_desc, p_desc, s_desc)) {
		return false;
	}
	servers_desc.push_back(p_desc);
	servers_desc.push_back(s_desc);
	return true;
}


bool DirectIOURLManagment::decodeServerDescriptionWithEndpoint(const std::string& server_description_endpoint,
															   std::string& server_description,
															   uint16_t& endpoint) {
    std::vector<std::string> tokens;
    
    if(!checkURL(server_description_endpoint)) {
        return false;
    }
    
    boost::algorithm::split(tokens, server_description_endpoint, boost::algorithm::is_any_of("|"), boost::algorithm::token_compress_on);
	
    server_description = tokens[0];
    endpoint = boost::lexical_cast<uint16_t>(tokens[1]);
    return true;
}
		}}}