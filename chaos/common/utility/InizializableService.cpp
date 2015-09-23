/*
 *	InizializableService.cpp
 *	!CHAOS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2013 INFN, National Institute of Nuclear Physics
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

#include <chaos/common/global.h>

#include <chaos/common/utility/InizializableService.h>

#define IS_LAPP LAPP_ << "[InizializableService]- "
#define IS_LDBG LDBG_ << "[InizializableService]- "
#define IS_LERR LERR_ << "[InizializableService]- "


using namespace chaos::common::utility;

/*!
 */
InizializableService::InizializableService() {
        //set the default value
    serviceState = service_state_machine::InizializableServiceType::IS_DEINTIATED;
}

/*!
 */
InizializableService::~InizializableService() {
    //if(state_machine) delete (state_machine);
}

//! Initialize instance
void InizializableService::init(void*) throw(chaos::CException) {
}

//! Deinit the implementation
void InizializableService::deinit() throw(chaos::CException) {
    
}
    //! Return the state
const uint8_t InizializableService::getServiceState() const {
    return serviceState;
}

bool InizializableService::initImplementation(InizializableService& impl, void *initData, const std::string & implName,  const std::string & domainString) {
    return initImplementation(&impl, initData, implName,  domainString);
}

bool InizializableService::deinitImplementation(InizializableService& impl, const std::string & implName,  const std::string & domainString) {
    return deinitImplementation(&impl, implName,  domainString);
}

/*!
 */
bool InizializableService::initImplementation(InizializableService *impl, void *initData, const std::string & implName,  const std::string & domainString)  {
    bool result = true;
    if(impl == NULL) throw CException(-1, "Implementation is null", domainString);
    try {
        IS_LAPP  << "Initializing " << implName;
        if(impl->state_machine.process_event(service_state_machine::EventType::init()) == boost::msm::back::HANDLED_TRUE) {
			try {
				impl->init(initData);
			}catch(CException& ex) {
				impl->InizializableService::state_machine.process_event(service_state_machine::EventType::deinit());
				throw ex;
			}
            
            impl->serviceState = impl->state_machine.current_state()[0];//service_state_machine::InizializableServiceType::IS_INITIATED;
        } else {
           throw CException(-2, "Service cant be initialized", domainString);
        }
        IS_LAPP  << implName << "Initialized";
    } catch (CException& ex) {
        IS_LERR  << "Error Initializing";
        DECODE_CHAOS_EXCEPTION_ON_LOG(IS_LERR, ex);
        impl->state_machine.process_event(service_state_machine::EventType::deinit());
        throw ex;
	} catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::bad_function_call> >& ex){
		IS_LERR  << "Error Deinitializing " << ex.what();
		throw CException(-3, std::string(ex.what()), std::string(__PRETTY_FUNCTION__));
	}
    return result;
}

/*!
 */
bool InizializableService::deinitImplementation(InizializableService *impl, const std::string & implName,  const std::string & domainString) {
    bool result = true;
    if(impl == NULL) throw CException(-1, "Implementation is null", domainString);
    try {
        IS_LAPP  << "Deinitializing " << implName;
        if(impl->state_machine.process_event(service_state_machine::EventType::deinit()) == boost::msm::back::HANDLED_TRUE) {
            impl->deinit();
        } else {
            throw CException(-2, "Service cant be deinitialized", domainString);
        }
        impl->serviceState = impl->state_machine.current_state()[0];//service_state_machine::InizializableServiceType::IS_DEINTIATED;
        IS_LAPP  << implName << "Deinitialized";
    } catch (CException& ex) {
        IS_LERR  << "Error Deinitializing";
        DECODE_CHAOS_EXCEPTION_ON_LOG(IS_LERR, ex);
        throw ex;
	} catch(boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::bad_function_call> >& ex){
		IS_LERR  << "Error Deinitializing " << ex.what();
		throw CException(-3, std::string(ex.what()), std::string(__PRETTY_FUNCTION__));
	}
    return result;
}
