/*
 *	HTTPUIInterface.cpp
 *	!CHAOS
 *	Created by Andrea Michelotti
 *
 *    	Copyright 2014 INFN, National Institute of Nuclear Physics
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

#include "HTTPUIInterface.h"
#include "HTTPWANInterfaceStringResponse.h"
#include <map>
#include <vector>
#include <chaos/common/utility/TimingUtil.h>

#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <boost/algorithm/string.hpp>

#include <json/json.h>

using namespace chaos;
using namespace chaos::common::data;
using namespace chaos::common::utility;
using namespace chaos::wan_proxy::wan_interface;
using namespace chaos::wan_proxy::wan_interface::http;
#define API_PREFIX_V1 "/api/v1"
#define API_PATH_REGEX_V1(p) API_PREFIX_V1 p

#define HTTWANINTERFACE_LOG_HEAD "["<<getName()<<"] - "

#define HTTWAN_INTERFACE_APP_ INFO_LOG(HTTPUIInterface)
#define HTTWAN_INTERFACE_DBG_ DBG_LOG(HTTPUIInterface)
#define HTTWAN_INTERFACE_ERR_ ERR_LOG(HTTPUIInterface)
static const boost::regex REG_API_URL_FORMAT(API_PATH_REGEX_V1("((/[a-zA-Z0-9_]+))*")); //"/api/v1((/[a-zA-Z0-9_]+))*"

std::map<std::string, ::driver::misc::ChaosController*> HTTPUIInterface::devs;



/**
 * The handlers below are written in C to do the binding of the C mongoose with
 * the C++ API
 */
static int event_handler(struct mg_connection *connection, enum mg_event ev) {
	if ((ev == MG_REQUEST)&& (connection->server_param != NULL)) {
		if((!strcmp(connection->uri,API_PREFIX_V1))&&((HTTPUIInterface *)connection->server_param)->handle(connection)){
			((HTTPUIInterface *)connection->server_param)->processRest(connection);

		} else {
			((HTTPUIInterface *)connection->server_param)->process(connection);
		}
		 return MG_TRUE;
	} else if (ev == MG_AUTH) {
	    return MG_TRUE;
	}
	return MG_FALSE;

}

static void flush_response(struct mg_connection *connection,
		AbstractWANInterfaceResponse *response) {
	CHAOS_ASSERT(connection && response)
					mg_send_status(connection, response->getCode());

	for(std::map<std::string, std::string>::const_iterator it = response->getHeader().begin();
			it != response->getHeader().end();
			it++){
		mg_send_header(connection, it->first.c_str(), it->second.c_str());
	}

	uint32_t body_len = 0;
	const char * body = response->getBody(body_len);
	mg_send_data(connection, body, body_len);
}

DEFINE_CLASS_FACTORY(HTTPUIInterface, AbstractWANInterface);
HTTPUIInterface::HTTPUIInterface(const string& alias):
				AbstractWANInterface(alias),
				run(false),
				thread_number(0) {

	info = new ::driver::misc::ChaosController();

}

HTTPUIInterface::~HTTPUIInterface() {}

//inherited method
void HTTPUIInterface::init(void *init_data) throw(CException) {
	//! forward message to superclass
	AbstractWANInterface::init(init_data);

	//clear in case last deinit fails
	http_server_list.clear();

	//check for parameter
	if(getParameter()[OPT_HTTP_PORT].isNull() ||
			!getParameter()[OPT_HTTP_PORT].isInt()) {
		std::string err = "Port for http interface as not be set!";
		HTTWAN_INTERFACE_ERR_ << err;
		throw chaos::CException(-1, err, __PRETTY_FUNCTION__);
	} else {
		service_port = getParameter()[OPT_HTTP_PORT].asInt();
	}

	if(getParameter()[OPT_HTTP_THREAD_NUMBER].isNull() ||
			!getParameter()[OPT_HTTP_THREAD_NUMBER].isInt()) {
		thread_number = 1;
	} else {
		thread_number = getParameter()[OPT_HTTP_THREAD_NUMBER].asInt();
	}

	HTTWAN_INTERFACE_APP_ << "HTTP server listen on port: " << service_port;
	HTTWAN_INTERFACE_APP_ << "HTTP server thread used: " << thread_number;

	//allcoate each server for every thread
	for(int idx = 1;
			idx <= thread_number;
			idx++) {
		struct mg_server *http_server = mg_create_server(this,event_handler);
		if(!http_server) {
			HTTWAN_INTERFACE_ERR_<< "cannot create server "<<idx;
			continue;
		}

		//configure server
		HTTWAN_INTERFACE_APP_ << " Thread " << idx << " allocated";
		std::string str_port = boost::lexical_cast<std::string>(service_port);
		mg_set_option(http_server, "listening_port", str_port.c_str());
		mg_set_option(http_server, "enable_keep_alive", "yes");
		mg_set_option(http_server, "enable_directory_listing", "false");
		HTTWAN_INTERFACE_APP_ << " Thread " << idx << " configured";
		//configure handler
//		mg_add_uri_handler(http_server, "/CU", event_handler);
	//	mg_add_uri_handler(http_server, "/MDS", event_handler);
		//		mg_server_do_i_handle(http_server, do_i_handle);
		HTTWAN_INTERFACE_APP_ << " Thread " << idx << " attached to handler";
		//add server to the list
		http_server_list.push_back(http_server);
		if(http_server_list.size()>1){
			mg_copy_listeners(http_server_list[0], http_server);
		}
	}
	if(!http_server_list.size()) throw chaos::CException(-1, "No http server has been instantiated", __PRETTY_FUNCTION__);

}

//inherited method
void HTTPUIInterface::start() throw(CException) {

	run = true;
	thread_index = 0;
	for(ServerListIterator it = http_server_list.begin();
			it != http_server_list.end();
			it++) {
		http_server_thread.add_thread(new boost::thread(boost::bind(&HTTPUIInterface::pollHttpServer, this, *it)));
	}
	sched_cu.start();
}

//inherited method
void HTTPUIInterface::stop() throw(CException) {
	run = false;
	http_server_thread.join_all();
}

//inherited method
void HTTPUIInterface::deinit() throw(CException) {
	for(ServerListIterator it = http_server_list.begin();
			it != http_server_list.end();
			it++) {
		mg_destroy_server(&(*it));
	}
	http_server_list.clear();
	//clear the service url
	service_port = 0;
}

void HTTPUIInterface::pollHttpServer(struct mg_server *http_server) {
	int current_index = ++thread_index;
	HTTWAN_INTERFACE_APP_ << "Entering http thread " << current_index;
	while (run) {
		mg_poll_server(http_server, 1000);
		//usleep(500);
	}
	HTTWAN_INTERFACE_APP_ << "Leaving http thread " << current_index;

}


void HTTPUIInterface::addDevice(std::string name, ::driver::misc::ChaosController*d) {

	devs[name] = d;

}

static std::map<std::string, std::string> mappify(std::string const& s)
				{
	std::map<std::string, std::string> m;
	std::vector<std::string> api_token_list0,api_token_list1;
	std::string key, val;
	std::istringstream iss(s);
	boost::algorithm::split(api_token_list0,s,boost::algorithm::is_any_of("&"),boost::algorithm::token_compress_on);
	for(std::vector<std::string>::iterator i=api_token_list0.begin();i!=api_token_list0.end();i++){
		boost::algorithm::split(api_token_list1,*i,boost::algorithm::is_any_of("="),boost::algorithm::token_compress_on);
		if(api_token_list1.size()==2){
			m[api_token_list1[0]]=api_token_list1[1];
		}

	}


	return m;
				}

int HTTPUIInterface::process(struct mg_connection *connection) {
	CHAOS_ASSERT(handler)
					int								err = 0;
	DEBUG_CODE(uint64_t                        execution_time_start = TimingUtil::getTimeStampInMicroseconds();)
	DEBUG_CODE(uint64_t                        execution_time_end = 0;)
	Json::Value						json_request;
	Json::Value						json_response;
	Json::StyledWriter				json_writer;
	Json::Reader					json_reader;
	HTTPWANInterfaceStringResponse	response("text/html");
	response.addHeaderKeyValue("Access-Control-Allow-Origin","*");
	::driver::misc::ChaosController* controller = NULL;
	//scsan for content type request
	const std::string method  = connection->request_method;
	const std::string url     = connection->uri;
	std::map<std::string, std::string> request;
	//	const std::string api_uri = url.substr(strlen(API_PREFIX_V1)+1);
	//const bool        json    = checkForContentType(connection,"application/json");

	//remove the prefix and tokenize the url
	if(method == "GET"){
		if(connection->query_string== NULL){
			HTTWAN_INTERFACE_ERR_<<"Bad query GET params";
			response.setCode(400);
			flush_response(connection, &response);
			return 1;
		}
		int size_query=strlen(connection->query_string)+2;
		char decoded[size_query];
		mg_url_decode(connection->query_string, size_query,decoded, size_query,0);
		HTTWAN_INTERFACE_DBG_<<"GET:"<<decoded;

		std::string query=decoded;
		request=mappify(query);
	} else if(method == "POST"){
		if(connection->content==NULL){
			HTTWAN_INTERFACE_ERR_<<"Bad query POST params";
			response.setCode(400);
			flush_response(connection, &response);
			return 1;
		}
		char decoded[connection->content_len +2];
		mg_url_decode(connection->content, connection->content_len,decoded, connection->content_len+2,0);
		std::string content_data(decoded, connection->content_len);
		HTTWAN_INTERFACE_DBG_<<"POST:"<<content_data;
		request=mappify(content_data);
	}
	std::string cmd, parm,dev_param;
	dev_param = request["dev"];
	cmd = request["cmd"];
	parm = request["parm"];
	std::string cmd_schedule = request["sched"];
	std::string cmd_prio = request["prio"];
	std::string cmd_mode = request["mode"];
	std::vector<std::string>dev_v;
	boost::split(dev_v,dev_param,boost::is_any_of(","));
	std::stringstream answer_multi;
	if(dev_param.size()==0){
		std::string ret;
		if(info->get(cmd,(char*)parm.c_str(),0,atoi(cmd_prio.c_str()),atoi(cmd_schedule.c_str()),atoi(cmd_mode.c_str()),0,ret)!=::driver::misc::ChaosController::CHAOS_DEV_OK){
			HTTWAN_INTERFACE_ERR_<<"An error occurred during get:"<<info->getJsonState();
			response.setCode(400);

		} else {
			response.setCode(200);
			response << ret;
		}
	} else {
		response.setCode(200);

		answer_multi<<"[";
		for(std::vector<std::string>::iterator idevname=dev_v.begin();idevname!=dev_v.end();idevname++){
			std::string ret;

			if ((*idevname).empty() || cmd.empty()) {
			  continue;
			}
			if(devs.count(*idevname)){
			 boost::mutex::scoped_lock l(devio_mutex);

			  controller = devs[*idevname];

			} else {
			  controller = new ::driver::misc::ChaosController();

			  if (controller == NULL) {
						response << "{}";
						response.setCode(400);
						HTTWAN_INTERFACE_ERR_<<"error creating Chaos Controller";
						flush_response(connection, &response);
						return 1;
					}
					if(controller->init(*idevname,DEFAULT_TIMEOUT_FOR_CONTROLLER)!=0){
						response << controller->getJsonState();
						HTTWAN_INTERFACE_ERR_<<"cannot init controller for "<<*idevname;
						response << "{}";
						response.setCode(400);
						delete controller;
						flush_response(connection, &response);
						return 1;
					}

					addDevice(*idevname,controller);
					sched_cu.add(*idevname,controller);


			}

			if(controller->get(cmd,(char*)parm.c_str(),0,atoi(cmd_prio.c_str()),atoi(cmd_schedule.c_str()),atoi(cmd_mode.c_str()),0,ret)!=::driver::misc::ChaosController::CHAOS_DEV_OK){
			  HTTWAN_INTERFACE_ERR_<<"An error occurred during get:"<<controller->getJsonState();
				}


				if((idevname+1) == dev_v.end()){
					answer_multi<<ret<<"]";
				}else {
					answer_multi<<ret<<",";
				}

			}
		response<<answer_multi.str();

		}




	flush_response(connection, &response);
	DEBUG_CODE(execution_time_end = TimingUtil::getTimeStampInMicroseconds();)
	DEBUG_CODE(uint64_t duration = execution_time_end - execution_time_start;)
	DEBUG_CODE(HTTWAN_INTERFACE_DBG_ << "Execution time is:" << duration*1.0/1000.0 << " ms";)

	return 1;//

}
bool HTTPUIInterface::handle(struct mg_connection *connection) {
    bool accepted = false;
    if(!(accepted = regex_match(connection->uri, REG_API_URL_FORMAT))) {
      //HTTWAN_INTERFACE_ERR_ << "URI:" << connection->uri << ", not accepted";
    }
	return accepted;
}
bool HTTPUIInterface::checkForContentType(struct mg_connection *connection,
                                           const std::string& type) {
    bool result = false;
    for(int idx = 0;
        idx < 30;
        idx++) {
        if(connection->http_headers[idx].name &&
           (std::strcmp(connection->http_headers[idx].name, "Content-Type") == 0)) {
            //we have content type
            result = (connection->http_headers[idx].value &&
                       (type.compare(connection->http_headers[idx].value) == 0));
            break;
        }

    }
    return result;
}

int HTTPUIInterface::processRest(struct mg_connection *connection) {
	CHAOS_ASSERT(handler)
	int								err = 0;
    DEBUG_CODE(uint64_t                        execution_time_start = TimingUtil::getTimeStampInMicroseconds();)
    DEBUG_CODE(uint64_t                        execution_time_end = 0;)
	Json::Value						json_request;
	Json::Value						json_response;
	Json::StyledWriter				json_writer;
	Json::Reader					json_reader;
	HTTPWANInterfaceStringResponse	response("application/json");

	//scsan for content type request
	const std::string method  = connection->request_method;
	const std::string url     = connection->uri;
	const std::string api_uri = url.substr(strlen(API_PREFIX_V1)+1);
	const bool        json    = checkForContentType(connection,
                                                    "application/json");

	//remove the prefix and tokenize the url
	std::vector<std::string> api_token_list;

	if(method == "GET"){
		boost::algorithm::split(api_token_list,
				connection->query_string,
							 boost::algorithm::is_any_of("&"),
							 boost::algorithm::token_compress_on);

		HTTWAN_INTERFACE_DBG_ <<"GET url:"<<url<<" api:"<<api_uri<<"content:"<<connection->content<<" query:"<<connection->query_string;

		 if((err = handler->handleCall(1,api_token_list,json_request,
		                                          response.getHeader(),
		                                          json_response))) {
		                HTTWAN_INTERFACE_ERR_ << "Error on api call :" << connection->uri;
		                //return the error for the api call
		                response.setCode(400);
		                json_response["error"] = err;
		                json_response["error_message"].append("Call Error");
		            }else{
		                //return the infromation of api call success
		                response.setCode(200);
		                json_response["error"] = 0;
		            }
		 response << json_writer.write(json_response);
		 	flush_response(connection, &response);
		     DEBUG_CODE(execution_time_end = TimingUtil::getTimeStampInMicroseconds();)
		     DEBUG_CODE(uint64_t duration = execution_time_end - execution_time_start;)
		     DEBUG_CODE(HTTWAN_INTERFACE_DBG_ << "Execution time is:" << duration << " microseconds";)
		 	return 1;//
	}
	boost::algorithm::split(api_token_list,
						 api_uri,
						 boost::algorithm::is_any_of("/"),
						 boost::algorithm::token_compress_on);

	//check if we havethe domain and api name in the uri and the content is json
	if(api_token_list.size()>= 2 &&
	   json) {
		std::string content_data(connection->content, connection->content_len);
        if(json_reader.parse(content_data, json_request)) {
            //print the received JSON document
            DEBUG_CODE(HTTWAN_INTERFACE_DBG_ << "Received JSON pack:" <<json_writer.write(json_request);)

            //call the handler
            if((err = handler->handleCall(1,
                                          api_token_list,
                                          json_request,
                                          response.getHeader(),
                                          json_response))) {
                DEBUG_CODE(HTTWAN_INTERFACE_ERR_ << "Error on api call :" << connection->uri <<
                           (content_data.size()? (" with message data: " + content_data):" with no message data");)
                //return the error for the api call
                response.setCode(400);
                json_response["error"] = err;
                json_response["error_message"].append("Call Error");
            }else{
                //return the infromation of api call success
                response.setCode(200);
                json_response["error"] = 0;
            }
        }else{
            response.setCode(400);
            json_response["error"] = -1;
            json_response["error_message"].append("Error parsing the json post data");
            DEBUG_CODE(HTTWAN_INTERFACE_ERR_ << "Error decoding the request:" << json_writer.write(json_response);)
        }


	} else {
		//return the error for bad json or invalid url
		response.setCode(400);
		response.addHeaderKeyValue("Content-Type", "application/json");
		json_response["error"] = -1;
        if(api_token_list.size()<2) {
            json_response["error_message"].append("The uri need to contains either the domain and name of the api ex: http[s]://host:port/api/vx/domain/name(/param)*");
        }
        if(!json) {
            json_response["error_message"].append("The content of the request need to be json");
        }
        DEBUG_CODE(HTTWAN_INTERFACE_ERR_ << "Error decoding the request:" <<json_writer.write(json_response);)
	}

	response << json_writer.write(json_response);
	flush_response(connection, &response);
    DEBUG_CODE(execution_time_end = TimingUtil::getTimeStampInMicroseconds();)
    DEBUG_CODE(uint64_t duration = execution_time_end - execution_time_start;)
    DEBUG_CODE(HTTWAN_INTERFACE_DBG_ << "Execution time is:" << duration << " microseconds";)
	return 1;//
}