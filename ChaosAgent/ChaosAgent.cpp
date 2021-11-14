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

#include "ChaosAgent.h"
#include "chaos_agent_constants.h"
#include <chaos/common/utility/FSUtility.h>
#include <chaos/common/metadata_logging/metadata_logging.h>
//#include <chaos_service_common/health_system/HealtManagerDirect.h>
#include <chaos/common/healt_system/HealtManager.h>
#include <chaos/common/io/SharedManagedDirecIoDataDriver.h>
#include <chaos/common/configuration/GlobalConfiguration.h>
#include "utility/ProcRestUtil.h"
#include <chaos/common/utility/Base64Util.h>

#define INFO    INFO_LOG(ChaosAgent)
#define ERROR   ERR_LOG(ChaosAgent)
#define DBG     DBG_LOG(ChaosAgent)

using namespace chaos::agent;
using namespace chaos::common::io;
using namespace chaos::common::utility;
using namespace chaos::common::healt_system;

chaos::WaitSemaphore ChaosAgent::wait_close_semaphore;
ChaosSharedPtr <chaos::agent::utility::ProcRestUtil> ChaosAgent::procRestUtil;

ChaosAgent::ChaosAgent() {}

ChaosAgent::~ChaosAgent() {}

void ChaosAgent::init(int argc, const char* argv[])  {
    ChaosCommon<ChaosAgent>::init(argc, argv);
}

void ChaosAgent::init(istringstream &initStringStream)  {
    ChaosCommon<ChaosAgent>::init(initStringStream);
}

void ChaosAgent::init(void *init_data)  {
    if((!GlobalConfiguration::getInstance()->hasOption(InitOption::OPT_NODEUID))||(GlobalConfiguration::getInstance()->getConfiguration()->getStringValue(InitOption::OPT_NODEUID).size()==0)){
			// change before NetworkBroker Initialization
        	nodeuid="ChaosAgent_"+chaos::GlobalConfiguration::getInstance()->getHostname();
			DBG << "'"<<InitOption::OPT_NODEUID <<"' not specified, setting uid to:"<<nodeuid;
		
			GlobalConfiguration::getInstance()->setNodeUID(nodeuid);
    	}
    ChaosCommon<ChaosAgent>::init(init_data);
    if (signal((int) SIGINT, ChaosAgent::signalHanlder) == SIG_ERR) {
        throw CException(-1, "Error registering SIGINT signal", __PRETTY_FUNCTION__);
    }
    
    if (signal((int) SIGQUIT, ChaosAgent::signalHanlder) == SIG_ERR) {
        throw CException(-2, "Error registering SIGERR signal", __PRETTY_FUNCTION__);
    }
    
    if (signal((int) SIGTERM, ChaosAgent::signalHanlder) == SIG_ERR) {
        throw CException(-3, "Error registering SIGTERM signal", __PRETTY_FUNCTION__);
    }
    /*if (signal((int) SIGKILL, ChaosAgent::signalHanlder) == SIG_ERR) {
        throw CException(-3, "Error registering SIGKILL signal", __PRETTY_FUNCTION__);
    }*/
    if(settings.working_directory.size() == 0) {
        settings.working_directory = FSUtility::getExecutablePath();
    }
    
    
    ChaosAgent::getInstance()->settings.agent_uid=nodeuid;
    //settings.agent_uid = CHAOS_FORMAT("ChaosAgent_%1%",%chaos::GlobalConfiguration::getInstance()->getLocalServerAddressAnBasePort());
    
    InizializableService::initImplementation(SharedManagedDirecIoDataDriver::getInstance(), NULL, "SharedManagedDirecIoDataDriver", __PRETTY_FUNCTION__);
    InizializableService::initImplementation(chaos::common::metadata_logging::MetadataLoggingManager::getInstance(), NULL, "MetadataLoggingManager", __PRETTY_FUNCTION__);

    StartableService::initImplementation(HealtManager::getInstance(), NULL, "HealthManager", __PRETTY_FUNCTION__);
   // StartableService::initImplementation(HealtManagerDirect::getInstance(), NULL, "HealtManagerDirect", __PRETTY_FUNCTION__);

    agent_register.reset(new AgentRegister(), "AgentRegister");
    CHECK_ASSERTION_THROW_AND_LOG((agent_register.get() != NULL), ERROR, -1, "AgentRegister instantiation failed");
    agent_register.init(NULL, __PRETTY_FUNCTION__);
#ifdef OLD_PROCESS_MANAGEMENT

    external_cmd_executor.reset(new external_command_pipe::ExternaCommandExecutor(), "ExternaCommandExecutor");
    CHECK_ASSERTION_THROW_AND_LOG((external_cmd_executor.get() != NULL), ERROR, -2, "ExternaCommandExecutor instantiation failed");
    external_cmd_executor.init(NULL, __PRETTY_FUNCTION__);
#else
    int restport=8071;
    if(GlobalConfiguration::getInstance()->hasOption(OPT_REST_PORT)) {
           restport=getGlobalConfigurationInstance()->getOption< int >(OPT_REST_PORT);
    }

    
    std::string script_path=getGlobalConfigurationInstance()->getOption< std::string >(OPT_SCRIPT_DIR) +"/"+ nodeuid;
    boost::filesystem::path p(script_path);
    ChaosAgent::getInstance()->settings.script_dir=script_path;
    ChaosAgent::getInstance()->settings.restport=restport;

    if ((boost::filesystem::exists(p) == false)) {
        try {
        if ((boost::filesystem::create_directories(p) == false) && ((boost::filesystem::exists(p) == false))) {
          ERROR << "cannot create directory:" << p;
          exit(1);
        } else {
          DBG << " CREATED DIR:" << p;
        }
        } catch(boost::filesystem::filesystem_error& e){
            std::stringstream ss;
            ss<< " Exception creating directory:"<<p<<" error:"<< e.what();
            logError(nodeuid,ss.str(),__PRETTY_FUNCTION__);
            
            ERROR<<ss.str();
      }
        // 
    }
    procRestUtil=ChaosMakeSharedPtr<utility::ProcRestUtil>(restport,script_path);
#endif
}

void ChaosAgent::start() {
    StartableService::startImplementation(HealtManager::getInstance(), "HealthManager", __PRETTY_FUNCTION__);
    ChaosCommon<ChaosAgent>::start();
    agent_register.start(__PRETTY_FUNCTION__);

 if (signal((int) SIGINT, ChaosAgent::signalHanlder) == SIG_ERR) {
        throw CException(-1, "Error registering SIGINT signal", __PRETTY_FUNCTION__);
    }
    
    if (signal((int) SIGQUIT, ChaosAgent::signalHanlder) == SIG_ERR) {
        throw CException(-2, "Error registering SIGERR signal", __PRETTY_FUNCTION__);
    }
    
    if (signal((int) SIGTERM, ChaosAgent::signalHanlder) == SIG_ERR) {
        throw CException(-3, "Error registering SIGTERM signal", __PRETTY_FUNCTION__);
    }

#ifndef OLD_PROCESS_MANAGEMENT
    DBG << " STARTING REST SERVER ON PORT:" << ChaosAgent::getInstance()->settings.restport;

    sleep(chaos::common::constants::HBTimersTimeoutinMSec/1000);
    procRestUtil->start(true); // start in background
#endif

    wait_close_semaphore.waitRaw();
    
    #ifndef OLD_PROCESS_MANAGEMENT
    LAPP_<<"Stop process";

    procRestUtil->stop();
    LAPP_<<"Kill processes";
    procRestUtil.reset();
    #endif
    stop();
    deinit();
    LAPP_<<"EXITING";

}
ChaosSharedPtr <chaos::agent::utility::ProcRestUtil> ChaosAgent::getProcessManager(){
    return procRestUtil;
}

void ChaosAgent::stop() {
    LAPP_<<"STOP";

    CHAOS_NOT_THROW(StartableService::stopImplementation(HealtManager::getInstance(), "HealthManager", __PRETTY_FUNCTION__););
    agent_register.stop(__PRETTY_FUNCTION__);
#ifndef OLD_PROCESS_MANAGEMENT
    procRestUtil->stop();
#endif
    CHAOS_NOT_THROW(ChaosCommon<ChaosAgent>::stop(););
}

void ChaosAgent::deinit() {
        LAPP_<<"DEINIT";

#ifdef OLD_PROCESS_MANAGEMENT

    CHAOS_NOT_THROW(external_cmd_executor.deinit(__PRETTY_FUNCTION__););
#endif
    CHAOS_NOT_THROW(StartableService::deinitImplementation(HealtManager::getInstance(), "HealthManager", __PRETTY_FUNCTION__););
    InizializableService::deinitImplementation(SharedManagedDirecIoDataDriver::getInstance(), "SharedManagedDirecIoDataDriver", __PRETTY_FUNCTION__);
    agent_register.deinit(__PRETTY_FUNCTION__);
    CHAOS_NOT_THROW(ChaosCommon<ChaosAgent>::deinit(););
}

void ChaosAgent::signalHanlder(int signal_number) {
    LAPP_<<"Signal caught:"<<signal_number;
   // procRestUtil->stop();
    wait_close_semaphore.notifyAll();

}


std::string ChaosAgent::scriptWorkingDir(std::string scriptname,std::string uid){
 std::string destdir;
    size_t res=scriptname.find_first_of(".",0);
    std::string base_dir=ChaosAgent::getInstance()->settings.script_dir;
    if(res==std::string::npos){
      destdir=base_dir+"/"+uid+std::string("/")+scriptname;
    } else {
      destdir=base_dir+"/"+uid+std::string("/")+scriptname.substr(0,res);
    }
    return destdir;
}
 std::string ChaosAgent::writeScript(const std::string& working_dir,const std::string& name,const std::string&content){
  try{
    DBG<<"creating directory \""<<working_dir<<"\"";
   if ((boost::filesystem::exists(working_dir) == false) &&
            (boost::filesystem::create_directories(working_dir) == false)) {
            std::stringstream ss;
            ss<<"cannot create directory:\""<<working_dir<<"\"";
            logError(nodeuid,ss.str(),__PRETTY_FUNCTION__);

         ERROR<<ss.str();
         return std::string();
  }
  } catch(boost::filesystem::filesystem_error& e){
            std::stringstream ss;
            ss<< " Filesystem Exception creating directory:"<<working_dir<<" error:"<< e.what();
            logError(nodeuid,ss.str(),__PRETTY_FUNCTION__);

    } catch(...){
        std::stringstream ss;

         ss<<"Exception cannot create directory:\""<<working_dir<<"\"";
        logError(nodeuid,ss.str(),__PRETTY_FUNCTION__);

        ERROR<<ss.str();
                                                    
         return std::string();

  }
  chaos::common::data::CDBufferUniquePtr towrite=Base64Util::decode(content);
  std::string fname=boost::filesystem::current_path().string()+"/"+working_dir+"/"+name;
    if(towrite.get()==NULL){
        ERROR<<"Empty content :\""<<content<<"\"";
         return std::string("");

    }
      DBG<<"creating file \""<<fname<<"\"";

  std::ofstream fs;
  fs.open(fname);
  if(!fs.fail()){
    fs.write(towrite->getBuffer(),towrite->getBufferSize());
    fs.close();
    DBG<<"written \""<<fname<<"\""<<towrite->getBufferSize()<<" bytes written";
    boost::filesystem::permissions(fname,boost::filesystem::owner_all);
    return fname;
  } else {
      std::stringstream ss;
      ss<<"cannot write file \""<<fname<<"\" of "<<towrite->getBufferSize()<<" bytes, error:"<< strerror(errno);
    logError(nodeuid,ss.str(),__PRETTY_FUNCTION__);

//throw CException(-10, ss.str(), __PRETTY_FUNCTION__);

  }

  return std::string("");
}
chaos::common::data::CDWUniquePtr ChaosAgent::checkAndPrepareScript(chaos::service_common::data::agent::AgentAssociation& it){
    chaos::common::data::CDWUniquePtr param;
    if(it.scriptID!=""){
        // probably is a script get the description.
        std::string path,wd;
        if((chaos::common::network::NetworkBroker::getInstance()->getMetadataserverMessageChannel()->getScriptDesc(it.scriptID,param)==0)&&param.get()&&param->hasKey("eudk_script_content")){
            chaos::common::data::CDWUniquePtr cl=param->clone();
            int sizeb=param->getStringValue("eudk_script_content").size();
            cl->removeKey("eudk_script_content");
            INFO <<" AGENT HAS TO START "<<it.scriptID<<" :"<<cl->getJSONString()<< " script size:"<<sizeb;
            wd=scriptWorkingDir(it.scriptID,it.associated_node_uid);
            if(wd.size()){
                path=writeScript(wd,it.scriptID,param->getStringValue("eudk_script_content"));
                if(path.size()>0){
                    it.working_dir=wd;
                    param->addStringValue("workdir",boost::filesystem::current_path().string()+"/"+wd);
                    INFO <<"WORKDIR PARAMETER:"<<param->getStringValue("workdir");

                    return param;
                }else {
                    // something went wrong creating directory/file
                    return chaos::common::data::CDWUniquePtr();        
                }     
            }      
    }
    }
    return param;
}
