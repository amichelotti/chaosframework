/*
 * Copyright 2021 INFN
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

#ifndef ChaosServiceToolkit_H
#define ChaosServiceToolkit_H
//#pragma GCC diagnostic ignored "-Woverloaded-virtual"

#include <boost/thread/condition.hpp>

#include <chaos/common/global.h>
#include <chaos/common/ChaosCommon.h>
#include <chaos/common/thread/WaitSemaphore.h>

#include <chaos/cu_toolkit/control_manager/ControlManager.h>
#include <chaos/cu_toolkit/control_manager/AbstractControlUnit.h>
#include <chaos/cu_toolkit/command_manager/CommandManager.h>
#include <chaos/cu_toolkit/driver_manager/DriverManager.h>

namespace chaos{
    namespace service_common {
       
        class ChaosServiceToolkit : public ChaosCommon<ChaosServiceToolkit>, public ServerDelegator {
            friend class common::utility::Singleton<ChaosServiceToolkit>;
            
            static WaitSemaphore waitCloseSemaphore;
            
            ChaosServiceToolkit();
            ~ChaosServiceToolkit();
            static void signalHanlder(int);
        protected:
            chaos::common::data::CDWUniquePtr nodeShutDown(chaos::common::data::CDWUniquePtr data);
        public:
            typedef ChaosLockGuard lock;
            //! C and C++ attribute parser
            /*!
             Specialized option for startup c and cpp program main options parameter
             */
            void init(int argc, const char* argv[]);
            //!stringbuffer parser
            /*
             specialized option for string stream buffer with boost semantics
             */
            void init(istringstream &initStringStream);
            void init(void *init_data);
            void start();
            void stop();
            void deinit();
            
            
        };
    }
}
//#pragma GCC visibility pop
#endif
