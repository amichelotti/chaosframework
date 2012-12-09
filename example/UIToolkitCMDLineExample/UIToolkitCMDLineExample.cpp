/*	
 *	UIToolkitCMDLineExample.cpp
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
#include <iostream>
#include <string>
#include <vector>
#include <chaos/common/global.h>
#include <chaos/common/cconstants.h>
#include <chaos/common/network/CNodeNetworkAddress.h>
#include <chaos/ui_toolkit/ChaosUIToolkit.h>
#include <chaos/ui_toolkit/LowLevelApi/LLRpcApi.h>
#include <chaos/ui_toolkit/HighLevelApi/HLDataApi.h>
#include <stdio.h>
#include <chaos/common/bson/bson.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
/*! \page page_example_uiex1 ChaosUIToolkit Example
 \section page_example_uiex1_sec A basic usage for the ChaosUIToolkit package
 
 \subsection page_example_uiex1_sec_sub1 Toolkit usage
 As in the CUToolkt, a developer can add a custom startup param
 \snippet example/UIToolkitCMDLineExample/UIToolkitCMDLineExample.cpp UIToolkit Init
 
 this example show how initialize the UIToolkit, acquire a basic channel, used it and deinit the toolkit.
 The UIToolkit is create around singleton pattern and the channel object are self managed by toolkit. So the first thing
 is to initializ the toolkit internal engine:
 
 \snippet example/UIToolkitCMDLineExample/UIToolkitCMDLineExample.cpp UIToolkit Init
 
 the argc, argv parameter are the common c and cpp main param.
 
 For comunicate with the chaos resurces, as example the metadataserver, a channel is needed. To instantiate a mds channel
 must be used the LLRpcApi api package. To achieve channel creation can be used the singleton instance for calling the method "getNewMetadataServerChannel"
 \snippet example/UIToolkitCMDLineExample/UIToolkitCMDLineExample.cpp UIToolkit ChannelCreation
 The returned pointer to MDSMessageChannel is interally managed, there is no need to deallocate it by and. When a channel is no more neede, the
 deallocation api must be called.
 
 At this point a request can be sent to metadata server and we must wait for the answer. For get all active devices we can use the method "getAllDeviceID"
 that get as param a vector of string, that wil be filled with all device id found
 \snippet example/UIToolkitCMDLineExample/UIToolkitCMDLineExample.cpp Datapack sent
 
 When all operation are finisched the UIToolkit ca be deinitialized, this operation will clean all pendi operation and hannel deallocation
 \snippet example/UIToolkitCMDLineExample/UIToolkitCMDLineExample.cpp UIToolkit Deinit
 */
using namespace std;
using namespace chaos;
using namespace chaos::ui;
using namespace bson;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::date_time;


#define OPT_ITERATION "iteration"
#define OPT_SLEEP_TIME "dac_sleep_time"

inline ptime utcToLocalPTime(ptime utcPTime){
	c_local_adjustor<ptime> utcToLocalAdjustor;
	ptime t11 = utcToLocalAdjustor.utc_to_local(utcPTime);
	return t11;
}

int main (int argc, char* argv[] )
{
    try {
        int err = 0;
        int iteration = 10;
        long sleep = 1000000;
        istringstream optionStream;
        string devID("test_2");
        vector<string> allDevice;
        posix_time::time_duration currentTime;
        
        //! [UIToolkit Attribute Init]
        ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_ITERATION, po::value<int>()->default_value(10), "Set the number of acquiring iteration");
        ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->addOption(OPT_SLEEP_TIME, po::value<long>()->default_value(1000000), "Set the number of microsecond between an acquisition and th eother");
        //! [UIToolkit Attribute Init]
        
        //! [UIToolkit Init]
        ChaosUIToolkit::getInstance()->init(argc, argv);
        //optionStream.str("metadata-server=testchaos:5000");
        //ChaosUIToolkit::getInstance()->init(optionStream);
        //! [UIToolkit Init]
        
        if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_ITERATION)){
            iteration = ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->getOption<int>(OPT_ITERATION);
        }
        
        if(ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->hasOption(OPT_SLEEP_TIME)){
            sleep = ChaosUIToolkit::getInstance()->getGlobalConfigurationInstance()->getOption<long>(OPT_SLEEP_TIME);
        }
        
        //! [UIToolkit ChannelCreation]
        CDeviceNetworkAddress deviceNetworkAddress;
        CUStateKey::ControlUnitState deviceState;
        //! [Datapack sent]
        
        DeviceController *controller = HLDataApi::getInstance()->getControllerForDeviceID(devID);
        if(!controller) {
             std::cout << "Error creating controller for:" << devID << std::endl;
        }
        //------------------------------------------------------------------------------------
        //List all attribute of dataset without use BSON
        vector<string> allOutAttrName;
        controller->getDeviceDatasetAttributesName(allOutAttrName, chaos::DataType::Output);
        std::cout << "Print all output attribute for the device to read:" << std::endl;
        for (int idx = 0; idx < allOutAttrName.size(); idx++) {
            std::cout << allOutAttrName[idx] << std::endl;
        }
        //------------------------------------------------------------------------------------
        /*
         get the state
         */
        err = controller->getState(deviceState);
        
        /*
         initialization of the device isntead form MDS
         */
        err = controller->initDevice();
        
        /*
         set the run schedule delay for the CU
         */
        err = controller->setScheduleDelay(1000000);

        //------------------------------------------------------------------------------------
        //send command for set attribute of dataset without use BSON
        err = controller->setInt32AttributeValue("points", 10);
        err = controller->setDoubleAttributeValue("gain_noise", 1.0);
        err = controller->setDoubleAttributeValue("gain", 2.0);
        err = controller->setDoubleAttributeValue("freq", 2.0);
        err = controller->setDoubleAttributeValue("phase", 2.0);
        //------------------------------------------------------------------------------------
        
        
        /*
         Start the control unit
         */
        err = controller->startDevice();
        
        
        //---------------------------------------------------------------------------------------------------------
        //this step is to use internal UIToolkit buffer logic
        controller->setupTracking();
        string key = "sinOutput";
        string key2 = "sinWave";
        controller->addAttributeToTrack(key);
        controller->addAttributeToTrack(key2);
        
        DataBuffer *intValue1Buff = controller->getBufferForAttribute(key);
        DataBuffer *tsBuffer = controller->getPtrBufferForTimestamp();
        PointerBuffer *binaryValueBuff = controller->getPtrBufferForAttribute(key2);
        
        for (int idx = 0; idx < iteration; idx++) {
            controller->fetchCurrentDeviceValue();
            
            if(intValue1Buff){
                int64_t hisotryToRead = intValue1Buff?intValue1Buff->getDimension()-intValue1Buff->getWriteBufferPosition():0;
                int64_t recentToRead = intValue1Buff?intValue1Buff->getWriteBufferPosition():0;
                std::cout << "History to read:" << hisotryToRead << std::endl;
                std::cout << "Recent to read:" << recentToRead << std::endl;
                
                double *bPtr = static_cast<double*>(intValue1Buff->getBasePointer());
                double *wPtr = static_cast<double*>(intValue1Buff->getWritePointer());
                
                std::cout << intValue1Buff->getWriteBufferPosition()<< std::endl;
                
                
                for (int idx = 0; idx < hisotryToRead-1; idx++) {
                    double *newbPtr=wPtr + idx;
                    std::cout << *newbPtr;
                }
                if(bPtr != wPtr){
                    for (int idx = 0; idx < recentToRead; idx++) {
                        double *newbPtr=bPtr + idx;
                        std::cout << *newbPtr;
                    }
                }
            }
            
            if(tsBuffer){
                int64_t *bPtr = static_cast<int64_t*>(tsBuffer->getBasePointer());
                int64_t *wPtr = static_cast<int64_t*>(tsBuffer->getWritePointer());
                int64_t *lastTimestamp = bPtr==wPtr?bPtr+tsBuffer->getDimension()-1:wPtr-1;
                if(lastTimestamp){
                    currentTime = boost::posix_time::milliseconds(*lastTimestamp);
                    std::cout << "Buffer received ts:" << utcToLocalPTime(EPOCH + currentTime) << std::endl;
                }
            }
            
            if(binaryValueBuff){ 
                int32_t tipedBufLen = 0;
                boost::shared_ptr<double> sinWavePtr = binaryValueBuff->getTypedPtr<double>(tipedBufLen);
                if(sinWavePtr.get()){
                    std::cout << "Buffer received len:" << tipedBufLen << std::endl;
                    for(int32_t idx = 0; idx < tipedBufLen; idx++){
                        std::cout << sinWavePtr.get()[idx];
                    }
                    
                    std::cout << std::endl;
                }
            }
            usleep(sleep);
            
        }
        controller->stopTracking();
        //---------------------------------------------------------------------------------------------------------
        
        /*
         stopping of the device instead form MDS
         */
        controller->stopDevice();
        
        controller->getState(deviceState);
        std::cout << "state " << deviceState << std::endl;
        
        /*
         deinit of the device instead form MDS
         */
        controller->deinitDevice();
        
        controller->getState(deviceState);
        std::cout << "state " << deviceState << std::endl;
        
        //! [UIToolkit Deinit]
        ChaosUIToolkit::getInstance()->deinit();
        //! [UIToolkit Deinit]
    } catch (CException& e) {
        std::cerr<< e.errorDomain << std::endl;
        std::cerr<< e.errorMessage << std::endl;
    }
    return 0;
}
