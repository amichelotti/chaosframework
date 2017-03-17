/*
 *	SafeAsyncCall.h
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 17/03/2017 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFramework_B0DC49BA_7D68_420A_A604_42071F01994C_SafeAsyncCall_h
#define __CHAOSFramework_B0DC49BA_7D68_420A_A604_42071F01994C_SafeAsyncCall_h

#include <boost/shared_ptr.hpp>

namespace chaos {
    namespace common {
        namespace utility {
            template<typename T>
            struct SafeAsyncCall {
                T function;
                SafeAsyncCall(const T& _function):
                function(_function){}
            };
        }
    }
}

#endif /* __CHAOSFramework_B0DC49BA_7D68_420A_A604_42071F01994C_SafeAsyncCall_h */