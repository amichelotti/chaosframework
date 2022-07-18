/*
 * Copyright 2022 INFN
 *
 * Andrea Michelotti
 * base class to process data coming from other nodes
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

#ifndef __CHAOSFramework__RTProcessAbstractControlUnit__
#define __CHAOSFramework__RTProcessAbstractControlUnit__

#include <chaos/cu_toolkit/control_manager/AbstractControlUnit.h>
#include <chaos/common/message/MessagePSDriver.h>

#define CONTROL_UNIT_SUBSCRIPTIONS_KEY "cudk_subscriptions"
#define CONTROL_UNIT_CONSUMER_GROUP_KEY "cudk_consumer_group"
#define CONTROL_UNIT_DISCARD_TOO_OLD_KEY "cudk_discard_too_old"

#define CREATE_HANDLER(class, type, abstractPointer)\
TDSObjectHandler<T, double> *typedHandler = NULL;\
typename TDSObjectHandler<T, double>::TDSHandler handlerPointer = objectMethodHandler;\
abstractPointer = typedHandler = new TDSObjectHandler<T, double>(objectPointer, handlerPointer);

namespace chaos {
    namespace cu {
		namespace control_manager {
                //forward declarations
            class ControManager;
            class AbstractExecutionUnit;
            
			class RTProcessAbstractControlUnit:
            public AbstractControlUnit {
				friend class ControlManager;
				friend class DomainActionsScheduler;
                friend class AbstractExecutionUnit;

                bool scheduler_run;
				uint64_t schedule_delay;
				int32_t discard_too_old;
				std::set<std::string> subscribe_nodes;
				std::string gid;
				chaos::common::message::consumer_uptr_t consumer;
				/*!
				 Define the control unit DataSet and Action into
				 a CDataWrapper
				 */
				void _defineActionAndDataset(chaos::common::data::CDataWrapper& setup_configuration);
                
				//! init rt control unit
				void init(void *initData);
				
				//! start rt control unit
				void start();
				
				//! stop rt control unit
				void stop();
				
				//! deinit rt control unit
				void deinit();
                //!redefine private for protection
                AbstractSharedDomainCache* _getAttributeCache();
				void parseSubscriptions(const std::string& load_params);
				void consumer_handler(chaos::common::message::ele_t&);
			protected:
                
                /*! default constructor
                 \param _control_unit_param is a string that contains parameter to pass during the contorl unit creation
                 \param _control_unit_drivers driver information
                 */
                RTProcessAbstractControlUnit(const std::string& _alternate_type,
                                      const std::string& _control_unit_id,
                                      const std::string& _control_unit_param);
                /*!
                 Parametrized constructor
                 \param _control_unit_id unique id for the control unit
                 \param _control_unit_param is a string that contains parameter to pass during the contorl unit creation
                 \param _control_unit_drivers driver information
                 */
                RTProcessAbstractControlUnit(const std::string& _alternate_type,
                                      const std::string& _control_unit_id,
                                      const std::string& _control_unit_param,
                                      const ControlUnitDriverList& _control_unit_drivers);
                
				//! schdule a run of the rt control unit
				
				//! set the dafult run schedule time intervall
				void setDefaultScheduleDelay(uint64_t _defaultScheduleDelay);
				
				/*
				 return the appropriate thread for the device
				 */
				inline void threadStartStopManagment(bool startAction);
				
                
               
			public:
				
				/*! default constructor
				 \param _control_unit_param is a string that contains parameter to pass during the contorl unit creation
				 \param _control_unit_drivers driver information
				 */
				RTProcessAbstractControlUnit(const std::string& _control_unit_id,
									  const std::string& _control_unit_param);
				/*!
				 Parametrized constructor
				 \param _control_unit_id unique id for the control unit
				 \param _control_unit_param is a string that contains parameter to pass during the contorl unit creation
				 \param _control_unit_drivers driver information
				 */
				RTProcessAbstractControlUnit(const std::string& _control_unit_id,
									  const std::string& _control_unit_param,
									  const ControlUnitDriverList& _control_unit_drivers);

				~RTProcessAbstractControlUnit();
				/**
				 * @brief This is called whenerver the CU receive a dataset from a subscribed node
				 * 
				 * @param key source of the message
				 * @param cd data
				 * @return >0 if processed, 0 if none, <0 if error
				 */
				virtual int unitProcessData(std::string& key,chaos::common::data::CDWUniquePtr& cd);
				void unitInit();
				void unitStart();
				void unitStop();
				void unitDeinit();
			};
		}
    }
}

#endif /* defined(__CHAOSFramework__RTProcessAbstractControlUnit__) */
