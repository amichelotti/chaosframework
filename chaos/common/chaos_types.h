/*
 *	chaos_types.h
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

#ifndef CHAOSFramework_chaos_types_h
#define CHAOSFramework_chaos_types_h
namespace chaos {
    namespace cu {
        namespace control_manager {
            namespace slow_command {
                
                //! Namespace for the slow command submisison rule
                namespace SubmissionRuleType {
                    /*!
                     * \enum SubmissionRule
                     * \brief Describe the state in which the command can be found
                     */
                    typedef enum SubmissionRule {
                        SUBMIT_AND_Stack    = 1,    /**< The new command wil stack the current executing command that consist in
                                                     install all implemented handler of the new one without touch the handler that are not implemented */
                        SUBMIT_AND_Kill     = 2,    /**< The new command will kill the current command, all hadnler ol killed one are erased and substituted */
                        SUBMIT_NORMAL       = 4     /**< The new command will waith the end of the current executed command and if an handler is implemented it is installed*/
                    } SubmissionRule;
                }
				
				/*!
				 \struct SandboxStat
				 Used for collect the statistic about current running command.
                 */
                typedef struct {
                    uint64_t lastCmdStepStart;	/**< Represent the time collected at the start of the scehduler step (before acquisition phase) */
                    uint64_t lastCmdStepTime;	/**< Represent the time collected at the end of the scehduler step (befor the sleep or pause of the thread) */
                } SandboxStat;
				
				namespace features {
					//! Namespace for the features of the slow command
					namespace FeaturesFlagTypes {
						/*!
						 * \enum FeatureFlag
						 * \brief The feature represent the custumoziation
						 * of the sandbox ad the moment the slowcommadn is installed
						 */
						typedef enum {
							FF_SET_SCHEDULER_DELAY      = 1,    /**< The command bring his own scheduler delay time */
							FF_SET_SUBMISSION_RETRY     = 2,    /**< The delay between a submiossion check and another, submiossion chek consist to check the current command running state to determinate
																 when the new command can be installed */
							FF_LOCK_USER_MOD			= 4
						} FeatureFlag;
					}
					
					typedef struct {
						
						bool lockedOnUserModification;
						
						//! Features flags
						/*!
						 One or more active flag from FeaturesFlagTypes::FeatureFlag
						 */
						uint8_t  featuresFlag;
						
						//features variable
						//! Command specific delay beetween two sequence of the commadn step (acquire->correlation)
						uint32_t featureSchedulerStepsDelay;
						
						//! Command specific delay between runninc command state and
						//! submission rule for install new command
						uint32_t featureSubmissionRetryDelay;
						
					} Features;
                }

            }
        }
    }
}

#endif
