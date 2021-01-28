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

#ifndef __CHAOSFramework__A4DDDA4_0D15_4B60_9D85_E983B249A7FE_ManagedDirectIODataDriver_h
#define __CHAOSFramework__A4DDDA4_0D15_4B60_9D85_E983B249A7FE_ManagedDirectIODataDriver_h
#if (defined(KAFKA_RDK_ENABLE) || defined(KAFKA_ASIO_ENABLE)) && (defined(HEALTH_ON_MESSAGE))
#include <chaos/common/io/IODirectIOPSMsgDriver.h>
#define IO_DIRECT  IODirectIOPSMsgDriver
#else
#include <chaos/common/io/IODirectIODriver.h>
#define IO_DIRECT IODirectIODriver
#endif


namespace chaos{
    namespace common {
        
        namespace message {
            class MDSMessageChannel;
        }
        
        namespace io {
            
            //!thia class extend the DirectIO IODataDriver
            /*!
             this class birng funcitonality for get best endpoint
             when the driver get initilized event and can be recofigured remotelly
             by metadata service
             */
            class ManagedDirectIODataDriver: public IO_DIRECT {

                message::MDSMessageChannel *mds_channel;
            public:
                ManagedDirectIODataDriver();
                ~ManagedDirectIODataDriver();
                void init(void *init_parameter);
                void deinit();
                void storeLogEntries(const std::string& key,
                                     ChaosStringVector log_entries);
            };
        }
    }
}

#endif /* __CHAOSFramework__A4DDDA4_0D15_4B60_9D85_E983B249A7FE_ManagedDirectIODataDriver_h */
