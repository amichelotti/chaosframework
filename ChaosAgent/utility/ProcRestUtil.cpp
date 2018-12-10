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

#include "ProcRestUtil.h"
#include "../ChaosAgent.h"
#include <chaos/common/global.h>
#include <chaos/common/configuration/GlobalConfiguration.h>

#include <boost/algorithm/string/replace.hpp>

#define READ   0
#define WRITE  1

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

using namespace chaos::common;

using namespace chaos::agent::utility;
using namespace chaos::service_common::data::agent;


std::string ProcRestUtil::normalizeName(const std::string& node_name) {
    std::string result = node_name;
    boost::replace_all(result,"/","_");
    return result;
}

void ProcRestUtil::launchProcess(const AgentAssociation& node_association_info) {
    int pid = 0;
    std::string exec_command;
    boost::filesystem::path init_file;
    boost::filesystem::path queue_file;
    try{
        if(checkProcessAlive(node_association_info) == true) return;
        exec_command = COMPOSE_NODE_LAUNCH_CMD_LINE(node_association_info);
        init_file = CHAOS_FORMAT("%1%/%2%", %INIT_FILE_PATH()%INIT_FILE_NAME(node_association_info));
        queue_file = CHAOS_FORMAT("%1%/%2%", %QUEUE_FILE_PATH()%NPIPE_FILE_NAME(node_association_info));
        
        boost::filesystem::path init_file_parent_path = INIT_FILE_PATH();
        if (boost::filesystem::exists(init_file_parent_path) == false &&
            boost::filesystem::create_directory(init_file_parent_path) == false) {
            throw chaos::CException(-1, CHAOS_FORMAT("Parent path %1% can't be created",%init_file_parent_path), __PRETTY_FUNCTION__);
        }
        
        boost::filesystem::path queue_file_parent_path = QUEUE_FILE_PATH();
        if (boost::filesystem::exists(queue_file_parent_path) == false &&
            boost::filesystem::create_directory(queue_file_parent_path) == false) {
            throw chaos::CException(-1, CHAOS_FORMAT("Queue path %1% can't be created",%queue_file_parent_path), __PRETTY_FUNCTION__);
        }
        
        //write configuration file
        std::ofstream init_file_stream;
        init_file_stream.open(init_file.string().c_str(), std::ofstream::trunc | std::ofstream::out);
        
        //enable log on console that will be redirected on named pipe
        init_file_stream << CHAOS_FORMAT("%1%=",%InitOption::OPT_LOG_ON_CONSOLE) << std::endl;
        
        //check for syslog setting of the agent that will be reflect on managed us
        if(GlobalConfiguration::getInstance()->hasOption(InitOption::OPT_LOG_ON_SYSLOG)) {
            init_file_stream << CHAOS_FORMAT("%1%=",%InitOption::OPT_LOG_ON_SYSLOG) << std::endl;
        }
        
        if(ChaosAgent::getInstance()->settings.enable_us_logging) {
            init_file_stream << CHAOS_FORMAT("%1%=",%InitOption::OPT_LOG_ON_FILE) << std::endl;
        }
        
        if(node_association_info.log_on_mds){
            init_file_stream << CHAOS_FORMAT("%1%=",%InitOption::OPT_LOG_ON_MDS) << std::endl;
        }
        
        init_file_stream << CHAOS_FORMAT("unit-server-alias=%1%",%node_association_info.associated_node_uid) << std::endl;
        
        //append metadata server from agent configuration
        VectorMetadatserver mds_vec = GlobalConfiguration::getInstance()->getMetadataServerAddressList();
        for(VectorMetadatserverIterator mds_it = mds_vec.begin(),
            end = mds_vec.end();
            mds_it != end;
            mds_it++) {
            init_file_stream << CHAOS_FORMAT("metadata-server=%1%",%mds_it->ip_port) << std::endl;
        }
        
        //append user defined paramenter
        init_file_stream.write(node_association_info.configuration_file_content.c_str(), node_association_info.configuration_file_content.length());
        init_file_stream.close();
        //create the named pipe
        //ProcRestUtil::createNamedPipe(queue_file.string());
        std::string upid=execProcess(exec_command);
        if(upid.size()>0){
            m_uid2upid[node_association_info.association_unique_id]=upid;
            LDBG_<<"New process created \""<<upid<<"\" unique id:"<<node_association_info.association_unique_id;
        }

    } catch(std::exception& ex) {
        throw ex;
    }
}

bool ProcRestUtil::checkProcessAlive(const AgentAssociation& node_association_info) {
    int pid;
    bool found = false;
    i_uid2upid_t t=m_uid2upid.find(node_association_info.association_unique_id);
    if(t==m_uid2upid.end()){
        LDBG_<<"process  with unique id:"<<node_association_info.association_unique_id<< " NOT PRESENT";
        return false;
    }
    ::restConsole::RestProcessManager::process_state_t proc=getState(t->second);
    
    return (proc==::restConsole::RestProcessManager::PROCESS_STARTED);
}

bool ProcRestUtil::quitProcess(const AgentAssociation& node_association_info,
                           bool kill) {
    bool found = false;
    i_uid2upid_t t=m_uid2upid.find(node_association_info.association_unique_id);
    if(t==m_uid2upid.end()){
        LDBG_<<"process  with unique id:"<<node_association_info.association_unique_id<< " NOT PRESENT";
        return false;
    }
    return (killProcess(t->second) == 0);
}
