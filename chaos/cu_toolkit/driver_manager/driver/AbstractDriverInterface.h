/*
 *	AbstractDriverInterface.h
 *	!CHAOS driver STUB
 *	Created by Andrea Michelotti
 *  Abstraction of a basic driver interface
 *    	Copyright 2020 INFN, National Institute of Nuclear Physics
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
#ifndef __ASTRACTDRIVERINTERFACE_H__
#define __ASTRACTDRIVERINTERFACE_H__
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/cu_toolkit/driver_manager/driver/DriverAccessor.h>
namespace chaos_driver=::chaos::cu::driver_manager::driver;

namespace chaos {
namespace cu {
namespace driver_manager{
namespace driver{

class AbstractDriverInterface {


protected:
    DrvMsg message;
    DriverAccessor* accessor;
    ChaosMutex io_mux;

public:
    AbstractDriverInterface(chaos_driver::DriverAccessor*_accessor):accessor(_accessor){if(_accessor==NULL)throw chaos::CException(-20,"invalid accessor",__FUNCTION__);};

    ~AbstractDriverInterface();
         /**
                     * @brief return a CDataWrapper (JSON) with the optional properties of a driver
                     * 
                     * @return properties
        */

   chaos::common::data::CDWUniquePtr getDrvProperties();


                    /**
                     * @brief set a CDataWrapper (JSON) with the optional properties 
                     * 
                     * @return properties
                     */

    chaos::common::data::CDWUniquePtr setDrvProperties(chaos::common::data::CDWUniquePtr &prop);
    std::string getUID() const {return accessor->getUID();}
    std::string getDriverName() const{return accessor->getDriverName();}
    std::string getLastError();
    void lock();
    void unlock();
    int try_lock();
};
}
}
}
}
#endif /* defined(__ControlUnitTest__DummyDriver__) */
