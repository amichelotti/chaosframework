/*
 *	ChaosWANProxy.cpp
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
#include "ChaosWANProxy.h"
#include "global_constant.h"
#include "DefaultWANInterfaceHandler.h"
#include "wan_interface/wan_interface.h"

#include <csignal>

#include <chaos/common/exception/CException.h>
#include <chaos/common/utility/StartableService.h>
#include <chaos/common/utility/ObjectFactoryRegister.h>

using namespace std;
using namespace chaos;
using namespace chaos::common::utility;
using namespace chaos::wan_proxy;
using namespace chaos::wan_proxy::persistence;
using boost::shared_ptr;

WaitSemaphore chaos::wan_proxy::ChaosWANProxy::waitCloseSemaphore;

#define LCND_LAPP LAPP_ << "[ChaosWANProxy] - "
#define LCND_LDBG LDBG_ << "[ChaosWANProxy] - " << __PRETTY_FUNCTION__ << " - "
#define LCND_LERR LERR_ << "[ChaosWANProxy] - " << __PRETTY_FUNCTION__ << "(" << __LINE__ << ") - "

ChaosWANProxy::ChaosWANProxy():
wan_interface_handler(NULL) {
	
}

ChaosWANProxy::~ChaosWANProxy() {
	
}

//! C and C++ attribute parser
/*!
 Specialized option for startup c and cpp program main options parameter
 */
void ChaosWANProxy::init(int argc, char* argv[]) throw (CException) {
	ChaosCommon<ChaosWANProxy>::init(argc, argv);
}
//!stringbuffer parser
/*
 specialized option for string stream buffer with boost semantics
 */
void ChaosWANProxy::init(istringstream &initStringStream) throw (CException) {
	ChaosCommon<ChaosWANProxy>::init(initStringStream);
}

/*
 *
 */
void ChaosWANProxy::init(void *init_data)  throw(CException) {
	std::string tmp_interface_name;
	try {
		ChaosCommon<ChaosWANProxy>::init(init_data);
		   
		if(!getGlobalConfigurationInstance()->hasOption(setting_options::OPT_INTERFACE_TO_ACTIVATE)) {
			throw CException(-1, "The interface protocol are mandatory", __PRETTY_FUNCTION__);
		}
		
		
		if (signal((int) SIGINT, ChaosWANProxy::signalHanlder) == SIG_ERR) {
			throw CException(-2, "Error registering SIGINT signal", __PRETTY_FUNCTION__);
		}
		
		if (signal((int) SIGQUIT, ChaosWANProxy::signalHanlder) == SIG_ERR) {
			throw CException(-3, "Error registering SIG_ERR signal", __PRETTY_FUNCTION__);
		}
		
		network_broker_service.reset(new NetworkBroker(), "NetworkBroker");
		network_broker_service.init(NULL, __PRETTY_FUNCTION__);
		
		persistence_driver.reset(new DefaultPersistenceDriver(network_broker_service.get()), "DefaultPresistenceDriver");
		persistence_driver.init(NULL, __PRETTY_FUNCTION__);
		persistence_driver->addServerList(setting.list_cds_server);
		
		//Allcoate the handler
		wan_interface_handler = new DefaultWANInterfaceHandler(persistence_driver.get());
		if(!wan_interface_handler) throw CException(-5, "Error instantiating wan interface handler", __PRETTY_FUNCTION__);

		((DefaultWANInterfaceHandler*)wan_interface_handler)->registerGroup();
		
		//start all proxy interface
		for(SettingStringListIterator it = setting.list_wan_interface_to_enable.begin();
			it != setting.list_wan_interface_to_enable.end();
			it++) {
			tmp_interface_name.clear();
			tmp_interface_name = *it + "WANInterface";
			wan_interface::AbstractWANInterface *tmp_interface_instance = ObjectFactoryRegister<wan_interface::AbstractWANInterface>::getInstance()->getNewInstanceByName(tmp_interface_name);
			if(!tmp_interface_instance) {
				LCND_LERR << "Error allocating " <<tmp_interface_name<< " wan interface";
				continue;
			}
			

			// try to initialize the implementation
			StartableService::initImplementation(tmp_interface_instance,
												 (void*)setting.parameter_wan_interfaces.c_str(),
												 tmp_interface_instance->getName(),
												 __PRETTY_FUNCTION__);
			//se the handler
			tmp_interface_instance->setHandler(wan_interface_handler);
			
			//add implemetnation to list
			wan_active_interfaces.push_back(tmp_interface_instance);
			
			LCND_LAPP << "Wan interface: " <<tmp_interface_instance->getName()<< " have been installed";
		}
		
	} catch (CException& ex) {
		DECODE_CHAOS_EXCEPTION(ex)
		exit(1);
	}
	//start data manager
}

/*
 *
 */
void ChaosWANProxy::start()  throw(CException) {
	//lock o monitor for waith the end
	try {
		//start network brocker
		network_broker_service.start(__PRETTY_FUNCTION__);
		
		//start all wan interface
		for(WanInterfaceListIterator it = wan_active_interfaces.begin();
			it != wan_active_interfaces.end();
			it++) {
			// try to start the implementation
			StartableService::startImplementation(*it,
												 (*it)->getName(),
												 __PRETTY_FUNCTION__);
		}
		
		//at this point i must with for end signal
		waitCloseSemaphore.wait();
	} catch (CException& ex) {
		DECODE_CHAOS_EXCEPTION(ex)
	}
	//execute the deinitialization of CU
	try{
		stop();
	} catch (CException& ex) {
		DECODE_CHAOS_EXCEPTION(ex)
	}
	
	try{
		deinit();
	} catch (CException& ex) {
		DECODE_CHAOS_EXCEPTION(ex)
	}
}

/*
 Stop the toolkit execution
 */
void ChaosWANProxy::stop()   throw(CException) {
	
	//start all wan interface
	for(WanInterfaceListIterator it = wan_active_interfaces.begin();
		it != wan_active_interfaces.end();
		it++) {
		// try to start the implementation
		CHAOS_NOT_THROW(StartableService::stopImplementation(*it,
															 (*it)->getName(),
															 __PRETTY_FUNCTION__);)
	}
	
	//stop network brocker
	network_broker_service.stop(__PRETTY_FUNCTION__);
	
	//endWaithCondition.notify_one();
	waitCloseSemaphore.unlock();
}

/*
 Deiniti all the manager
 */
void ChaosWANProxy::deinit()   throw(CException) {
	//deinit all wan interface
	for(WanInterfaceListIterator it = wan_active_interfaces.begin();
		it != wan_active_interfaces.end();
		it++) {
		// try to deinit the implementation
		CHAOS_NOT_THROW(StartableService::deinitImplementation(*it,
															   (*it)->getName(),
															   __PRETTY_FUNCTION__);)
		//delete it
		delete(*it);
	}
	
	//clear the vector
	wan_active_interfaces.clear();
	
	if(wan_interface_handler) {
		delete(wan_interface_handler);
		wan_interface_handler = NULL;
	}
	
	persistence_driver.deinit(__PRETTY_FUNCTION__);
	
	//deinit network brocker
	network_broker_service.deinit(__PRETTY_FUNCTION__);
}

/*
 *
 */
void ChaosWANProxy::signalHanlder(int signalNumber) {
	//lock lk(monitor);
	//unlock the condition for end start method
	//endWaithCondition.notify_one();
	waitCloseSemaphore.unlock();
}
