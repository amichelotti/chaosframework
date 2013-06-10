/*
 *	AbstractDriver.h
 *	!CHOAS
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

#ifndef __CHAOSFramework__AbstractDriver__
#define __CHAOSFramework__AbstractDriver__

#include <string>
#include <chaos/cu_toolkit/ControlManager/driver/DriverGlobal.h>

namespace chaos{
    namespace cu {
        namespace cm {
            namespace driver {
            
                class DriverAccessor;
                
                
                    //! !CHAOS Driver abstract class
                /*!
                    This represent the base class for all driver in !CHOAS. For standardize the comunicacetion 
                    a message queue is used for receive DrvMsg pack.
                 */
                class AbstractDriver {
                    
                    std::string driverUUID;
                    
                    //! command queue used for receive DrvMsg pack
                    boost::interprocess::message_queue *commandQueue;
                    
                    //!Private constructor
                    AbstractDriver();
                    
                    //!Private destructor
                    virtual ~AbstractDriver();
                    
                    //! Wait the new command and broadcast it
                    /*!
                        This method waith for the next command, broadcast it
                        and check if the opcode is the "end of work" opcode, 
                        in this case it will quit.
                     */
                    void scanForMessage();
                    
                public:
                    //! Create a new accessor
                    /*!
                        A new accessor is allocate. In the allocation process
                        the message queue for comunicating with this driver is
                        allocated.
                     */
                    bool getNewAccessor(DriverAccessor **newAccessor);

                    //! Execute a command
                    /*!
                        The driver implementation must use the opcode to recognize the
                        command to execute and then write it on th ememory allocate
                        by the issuer of the command.
                     */
                    virtual void execOpcode(DrvMsgPtr cmd) = 0;
                };
                
                
            }
        }
    }
}

#endif /* defined(__CHAOSFramework__AbstractDriver__) */
