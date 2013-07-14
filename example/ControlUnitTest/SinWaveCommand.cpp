/*
 *	SinWaveCommand.h
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
#include "SinWaveCommand.h"
using namespace chaos;
namespace slowcmd = chaos::cu::control_manager::slow_command;

SinWaveCommand::SinWaveCommand():rng((const uint_fast32_t) time(0) ),one_to_hundred( -100, 100 ),randInt(rng, one_to_hundred) {
    
}

SinWaveCommand::~SinWaveCommand() {
    
}

// return the implemented handler
uint8_t SinWaveCommand::implementedHandler() {
    return  slowcmd::HandlerType::HT_Set |
            slowcmd::HandlerType::HT_Acquisition |
            slowcmd::HandlerType::HT_Correlation;
}

// Start the command execution
uint8_t SinWaveCommand::setHandler(chaos::CDataWrapper *data) {
    srand((unsigned)time(0));
    PI = acos((long double) -1);
    sinevalue = NULL;
    setWavePoint(30);
    freq = 1.0;
    gain = 5.0;
    phase = 0.0;
    bias = 0.0;
    gainNoise = 0.5;
    return 0;
}

// Aquire the necessary data for the command
/*!
 The acquire handler has the purpose to get all necessary data need the by CC handler.
 \return the mask for the runnign state
 */
uint8_t SinWaveCommand::acquireHandler() {
    uint8_t state = slowcmd::RunningStateType::RS_Stack;
    CDataWrapper *acquiredData = getNewDataWrapper();
    if(!acquiredData) return state;
    
    //put the messageID for test the lost of package
    acquiredData->addInt32Value("id", ++messageID);
    computeWave(acquiredData);
    //adding some interesting random data
    
    //submit acquired data
    pushDataSet(acquiredData);
    
    return state;
}

// Correlation and commit phase
uint8_t SinWaveCommand::ccHandler() {
    return slowcmd::RunningStateType::RS_Stack;
}

void SinWaveCommand::computeWave(CDataWrapper *acquiredData) {
    if(sinevalue == NULL) return;
    double interval = (2*PI)/points;
    boost::mutex::scoped_lock lock(pointChangeMutex);
    for(int i=0; i<points; i++){
        sinevalue[i] = (gain*sin((interval*i)+phase)+(((double)randInt()/(double)100)*gainNoise)+bias);
    }
    acquiredData->addBinaryValue("sinWave", (char*)sinevalue, (int32_t)sizeof(double)*points);
}

/*
 */
void SinWaveCommand::setWavePoint(const int32_t& newNumberOfPoints) {
    boost::mutex::scoped_lock lock(pointChangeMutex);
    int32_t tmpNOP = newNumberOfPoints;
    if(tmpNOP < 1) tmpNOP = 0;
    
    if(!tmpNOP){
        if(sinevalue){
            free(sinevalue);
            sinevalue = NULL;
        }
    }else{
        size_t byteSize = sizeof(double) * tmpNOP;
        double* tmpPtr = (double*)realloc(sinevalue, byteSize);
        if(tmpPtr) {
            sinevalue = tmpPtr;
            memset(sinevalue, 0, byteSize);
        }else{
            //memory can't be enlarged so pointer ramin the same
            //so all remain unchanged
            tmpNOP = points;
        }
    }
    points = tmpNOP;
}