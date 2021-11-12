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
#include <chaos/common/healt_system/HealtManager.h>
#include <chaos/common/io/SharedManagedDirecIoDataDriver.h>
#include <chaos/common/log/LogManager.h>
#include <chaos/common/metadata_logging/MetadataLoggingManager.h>
#include "ChaosServiceToolkit.h"
#include <chaos/cu_toolkit/control_manager/script/api/api.h>
//#include <chaos/cu_toolkit/data_manager/DataManager.h>
using namespace std;
using namespace chaos::cu;
using namespace chaos::cu::data_manager;
using namespace chaos::common::io;
using namespace chaos::common::data;
using namespace chaos::common::utility;
using namespace chaos::common::healt_system;
using namespace chaos::common::metadata_logging;
using namespace chaos::service_common;
//boost::mutex ChaosServiceToolkit::monitor;
//boost::condition ChaosServiceToolkit::endWaithCondition;
chaos::WaitSemaphore ChaosServiceToolkit::waitCloseSemaphore;
void crit_err_hdlr(int sig_num, siginfo_t *info, void *ucontext);

ChaosServiceToolkit::ChaosServiceToolkit() {
  

  
}

ChaosServiceToolkit::~ChaosServiceToolkit() {
}

//! C and C++ attribute parser
/*!
 Specialized option for startup c and cpp program main options parameter
 */
void ChaosServiceToolkit::init(int argc, const char* argv[]) {
  ChaosCommon<ChaosServiceToolkit>::init(argc, argv);
}
//!stringbuffer parser
/*
 specialized option for string stream buffer with boost semantics
 */
void ChaosServiceToolkit::init(istringstream& initStringStream) {
  ChaosCommon<ChaosServiceToolkit>::init(initStringStream);
}

/*
 *
 */
void ChaosServiceToolkit::init(void* init_data) {
  try {
    ChaosCommon<ChaosServiceToolkit>::init(init_data);

    if (signal((int)SIGINT, ChaosServiceToolkit::signalHanlder) == SIG_ERR) {
      LERR_ << "SIGINT Signal handler registration error";
    }
#ifndef _WIN32
    if (signal((int)SIGQUIT, ChaosServiceToolkit::signalHanlder) == SIG_ERR) {
      LERR_ << "SIGQUIT Signal handler registration error";
    }
#endif
    if (signal((int)SIGTERM, ChaosServiceToolkit::signalHanlder) == SIG_ERR) {
      LERR_ << "SIGTERM Signal handler registration error";
    }
#ifndef _WIN32
    struct sigaction sigact;
    std::memset(&sigact, 0, sizeof(struct sigaction));
    sigact.sa_sigaction = crit_err_hdlr;
    sigact.sa_flags     = SA_RESTART | SA_SIGINFO;
        if (sigaction(SIGABRT, &sigact, (struct sigaction *)NULL) != 0) {
            LERR_ << "error setting signal handler for SIGSEGV";
        }
        if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0) {
            LERR_ << "error setting signal handler for SIGSEGV";
        }

#endif

    InizializableService::initImplementation(SharedManagedDirecIoDataDriver::getInstance(), NULL, "SharedManagedDirecIoDataDriver", __PRETTY_FUNCTION__);

    if (GlobalConfiguration::getInstance()->hasOption(InitOption::OPT_LOG_ON_MDS) &&
        GlobalConfiguration::getInstance()->hasOption(InitOption::OPT_NODEUID)) {
        nodeuid=GlobalConfiguration::getInstance()->getNodeUID();
      chaos::common::log::LogManager::getInstance()->addMDSLoggingBackend(nodeuid);
    }

    //force first allocation of metadata logging
    if (GlobalConfiguration::getInstance()->getMetadataServerAddressList().size()) {
      //we can initilize the logging manager
      InizializableService::initImplementation(chaos::common::metadata_logging::MetadataLoggingManager::getInstance(), NULL, "MetadataLoggingManager", __PRETTY_FUNCTION__);
    }
    StartableService::initImplementation(HealtManager::getInstance(), NULL, "HealtManager", __PRETTY_FUNCTION__);
   // StartableService::initImplementation(DataManager::getInstance(), NULL, "DataManager", "ChaosServiceToolkit::init");



    LAPP_ << "!CHAOS Common Service Initialized";

  } catch (CException& ex) {
    DECODE_CHAOS_EXCEPTION(ex)
    exit(1);
  }
  //start data manager
}

/*
 *
 */
void ChaosServiceToolkit::start() {
  try {
    ChaosCommon<ChaosServiceToolkit>::start();
    LAPP_ << "Starting !CHAOS Common Service";
    StartableService::startImplementation(HealtManager::getInstance(), "HealtManager", __PRETTY_FUNCTION__);
   // StartableService::startImplementation(DataManager::getInstance(), "DataManager", "ChaosServiceToolkit::start");
    LAPP_ << "-----------------------------------------";
    LAPP_ << "!CHAOS Common Service Started";
    LAPP_ << "-----------------------------------------";
    //at this point i must with for end signal
    waitCloseSemaphore.wait();
    LAPP_ << "------ ORDERED EXIT---";
    sleep(1);
  } catch (CException& ex) {
    DECODE_CHAOS_EXCEPTION(ex);
  } catch (...) {
    LERR_ << "unexpected exeception";
  }
  //execute the stop and the deinitialization of the toolkit
  stop();
  deinit();
}

/*
 Stop the toolkit execution
 */
void ChaosServiceToolkit::stop() {
  LAPP_ << "Stopping !CHAOS Common Service";
  //CHAOS_NOT_THROW(StartableService::stopImplementation(DataManager::getInstance(), "DataManager", "ChaosServiceToolkit::stop"););
  CHAOS_NOT_THROW(StartableService::stopImplementation(HealtManager::getInstance(), "HealtManager", __PRETTY_FUNCTION__););
  ChaosCommon<ChaosServiceToolkit>::stop();
}

/*
 Deiniti all the manager
 */
void ChaosServiceToolkit::deinit() {
  LAPP_ << "Deinitilizzating !CHAOS Common Service";
  
//  CHAOS_NOT_THROW(StartableService::deinitImplementation(DataManager::getInstance(), "DataManager", "ChaosServiceToolkit::deinit"););
  CHAOS_NOT_THROW(InizializableService::deinitImplementation(MetadataLoggingManager::getInstance(), "MetadataLoggingManager", __PRETTY_FUNCTION__););
  CHAOS_NOT_THROW(StartableService::deinitImplementation(HealtManager::getInstance(), "HealtManager", __PRETTY_FUNCTION__););
  CHAOS_NOT_THROW(InizializableService::deinitImplementation(SharedManagedDirecIoDataDriver::getInstance(), "SharedManagedDirecIoDataDriver", __PRETTY_FUNCTION__););
  ChaosCommon<ChaosServiceToolkit>::deinit();
  LAPP_ << "-----------------------------------------";
  LAPP_ << "!CHAOS Common Service deinitialized  ";
  LAPP_ << "-----------------------------------------";
}


/*
 *
 */
void ChaosServiceToolkit::signalHanlder(int signalNumber) {
  //lock lk(monitor);
  //unlock the condition for end start method
  //endWaithCondition.notify_one();
  //waitCloseSemaphore.unlock();

  if ((signalNumber == SIGABRT) || (signalNumber == SIGSEGV)) {
    LAPP_ << "INTERNAL ERROR, please provide log, Catch SIGNAL: " << signalNumber;
    sigignore(signalNumber);
    waitCloseSemaphore.notifyAll();
    sleep(5);
    exit(1);
    //throw CFatalException(signalNumber,"trapped signal",__PRETTY_FUNCTION__);
  } else if ((signalNumber == SIGTERM) || (signalNumber == SIGINT)) {
    sigignore(signalNumber);
    LAPP_ << "CATCH Interrupting signal  "<<signalNumber<<" exiting ...";
    waitCloseSemaphore.notifyAll();
    sleep(5);
    exit(0);
  } else {
    LAPP_ << "CATCH SIGNAL " << signalNumber;

    //exit(0);
  }
  sleep(1);
}

CDWUniquePtr ChaosServiceToolkit::nodeShutDown(chaos::common::data::CDWUniquePtr data) {
    CDWUniquePtr ret=ChaosCommon<ChaosServiceToolkit>::nodeShutDown(MOVE(data));

    signal(SIGABRT,SIG_DFL);
    signal(SIGSEGV,SIG_DFL);
    signal(SIGTERM,SIG_DFL);
    signal(SIGINT,SIG_DFL);

    waitCloseSemaphore.unlock();
    return ret;
}
