/*
 *	LFDataCache.cpp
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

#include <chaos/common/global.h>
#include <chaos/common/data/cache/LFDataCache.h>
#include <boost/interprocess/detail/atomic.hpp>

using namespace chaos::common::data::cache;

LFDataCache::LFDataCache(memory::ManagedMemory *_memoryPool):writeIndex(0),readIndex(1),memoryPool(_memoryPool),garbageableSlab(1) {
    
    //clear the array
    rwPtr[0] = rwPtr[1] = NULL;
}

LFDataCache::~LFDataCache() {
}

//! set the readable index of the ptr array
void LFDataCache::swapRWIndex() {
    //swap read and write
    readIndex.store(writeIndex, boost::memory_order_release);
    
    //compute new write index
    writeIndex ^= 0x00000001;

    // put old readeable slba into garbageable index
    garbageableSlab.push(rwPtr[writeIndex]);
    
    // alloc new slab info
    rwPtr[writeIndex] = makeNewChachedInfoPtr();
}

inline SlbCachedInfoPtr LFDataCache::makeNewChachedInfoPtr() {
    SlbCachedInfoPtr result = static_cast<SlbCachedInfoPtr>(memoryPool->allocate(slabRequiredSize, slabID));
    if(!result) return NULL;
    //clear all memory
    //memset(result, 0, slabRequiredSize);
    result->references = 1;
    //result->valPtr = (SlbCachedInfoPtr)((char*)result+sizeof(boost::uint32_t));
    result->valPtr = (void*)((char*)result+sizeof(SlbCachedInfo));
    return result;
}

void LFDataCache::garbageCache() {
    int counter = 0;
    bool needToBeGarbaged = false;
    volatile boost::uint32_t *mem;
	boost::uint32_t oldMem = 0;
	boost::uint32_t oldValue = 0;

    
    if(garbageableSlab.empty()) return;
    
    //cicle all slab to make it garbaged
    SlbCachedInfoPtr tmpPtr = NULL;
    do {
        if((needToBeGarbaged=garbageableSlab.pop(tmpPtr))){
            counter++;
            if(tmpPtr->references==1) {
                //try to put at 0 and garbage
                mem = &tmpPtr->references;
                oldMem = *mem;
                //increment the value with cas operation
                oldValue = boost::interprocess::ipcdetail::atomic_cas32(mem, *mem - 1, oldMem);
                
                if(oldValue == oldMem) {
                    //i can garbage the slab
                    memoryPool->deallocate(tmpPtr, slabRequiredSize, slabID);
                }
            }
        }
    } while(needToBeGarbaged);
}

//! Initialize the channel cache
void LFDataCache::init(const char *name, uint32_t maxLength) {
    CHAOS_ASSERT(memoryPool)
    this->maxLength = maxLength;
    uint32_t structSize = (uint32_t)sizeof(SlbCachedInfo);
    slabRequiredSize = structSize + maxLength;
    
    // retrive the rigth slab class info
    slabID = memoryPool->getSlabIdBySize(slabRequiredSize);
    
    //write the default writebla slab, clear all and call swapRWIndex to make it readable
    rwPtr[0] = makeNewChachedInfoPtr();
    rwPtr[1] = makeNewChachedInfoPtr();
}

//! Set (and cache) the new value of the channel
void LFDataCache::updateValue(const void* value, uint32_t legth) {
    SlbCachedInfoPtr tmpPtr = rwPtr[writeIndex];
    //copy the value into cache
    
    memcpy(tmpPtr->valPtr, value, legth==0?maxLength:legth);
    
    //swap the wPtr with rPtr
    swapRWIndex();
}

SlbCachedInfoPtr LFDataCache::getCurrentCachedPtr() {
    SlbCachedInfoPtr result = NULL;
    volatile boost::uint32_t *mem;
    boost::uint32_t oldMem, oldValue;
    
    //get the old reference count from current RPtr
    do {
        //get info ptr
        result  = rwPtr[readIndex.load(boost::memory_order_consume)];
        //get ref pointer
        mem = &result->references;
        //get the old value
        oldMem = *mem;
        //if 0 is not usable
        if(oldMem == 0) continue;
        //increment the value with cas operation
        oldValue = boost::interprocess::ipcdetail::atomic_cas32(mem, *mem + 1, oldMem);
        
        //check if old value is the same of the one memorized early
    } while (oldValue != oldMem);
    
    //we have suceed to udpate the reference count without noone has modified it
    return result;
}

void LFDataCache::fillAccessorWithCurrentValue(ChannelValueAccessor&  accessorPtr) {
    accessorPtr.reset(getCurrentCachedPtr());
}
