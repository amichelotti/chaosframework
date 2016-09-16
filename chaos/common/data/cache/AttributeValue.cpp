/*
 *	AttributesValue.cpp
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

#include <chaos/common/global.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/data/cache/AttributeValue.h>

#include <boost/lexical_cast.hpp>

#include <assert.h>

#define AVLAPP_ LAPP_ << "[AttributeValue -" << "] - "
#define AVLDBG_ LDBG_ << "[AttributeValue -" << "] " << __PRETTY_FUNCTION__ << " - "
#define AVLERR_ LERR_ << "[AttributeValue -" << "] " << __PRETTY_FUNCTION__ << "(" << __LINE__ << ") - "

using namespace chaos::common::data;
using namespace chaos::common::data::cache;

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
AttributeValue::AttributeValue(const std::string& _name,
                               uint32_t _index,
                               uint32_t _size,
                               chaos::DataType::DataType _type):
value_buffer(NULL),
size(_size),
name(_name),
index(_index),
buf_size(0),
type(_type),
sharedBitmapChangedAttribute(NULL){
    
    if(size) {
        //elarge memory buffer and clear it
        if(!setNewSize(size, true)) {
            AVLERR_ << "error allocating current_value memory";
            assert(0);
        }
    }
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
AttributeValue::~AttributeValue() {
    if(value_buffer) {
        buf_size =0;
        size=0;
        free(value_buffer);
    }
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::grow(uint32_t value_size) {
    if(size<value_size){
        if(setNewSize(value_size,false)==false){
            return false;
        }
    }
    return true;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::setValue(const void* value_ptr,
                              uint32_t value_size,
                              bool tag_has_changed) {
    
    if(!grow(value_size)) return false;
    
    if(value_size==0)
        return true;
    
    CHAOS_ASSERT(value_buffer)
    
    //copy the new value
    std::memcpy(value_buffer, value_ptr, value_size);
    
    //set the relative field for set has changed
    if(tag_has_changed) sharedBitmapChangedAttribute->set(index);
    return true;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/

bool AttributeValue::setValue(const CDataVariant& attribute_value,
                              bool tag_has_changed) {
    CHAOS_ASSERT(value_buffer)
    
    if(type != attribute_value.getType()) return false;
    switch (attribute_value.getType()) {
        case DataType::TYPE_DOUBLE: {
            bool bv = attribute_value.asBool();
            //copy string to buffer
            std::memcpy(value_buffer,
                        &bv,
                        sizeof(bool));
            break;
        }
        case DataType::TYPE_INT32: {
            int32_t i32v = attribute_value.asInt32();
            //copy string to buffer
            std::memcpy(value_buffer,
                        &i32v,
                        sizeof(int32_t));
            break;
        }
            
        case DataType::TYPE_INT64: {
            int64_t i64v = attribute_value.asInt64();
            //copy string to buffer
            std::memcpy(value_buffer,
                        &i64v,
                        sizeof(int64_t));
            break;
        }

        case DataType::TYPE_STRING: {
            const std::string value = attribute_value.asString();
            if(!grow((uint32_t)value.size())) return false;
            //copy string to buffer
            std::memcpy(value_buffer,
                        value.c_str(),
                        value.size());
            break;
        }
       
        case DataType::TYPE_BYTEARRAY:{
            const CDataBuffer * byte_array_value = attribute_value.asCDataBuffer();
            if(!grow(byte_array_value->getBufferSize())) return false;
            //copy buffer to buffer
            std::memcpy(value_buffer,
                        byte_array_value->getBuffer(),
                        byte_array_value->getBufferSize());
            break;
        }
        default:
            break;
    }

    //set the relative field for set has changed
    if(tag_has_changed) sharedBitmapChangedAttribute->set(index);
    return true;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::setNewSize(uint32_t _new_size,
                                bool clear_mem) {
    bool result = true;
    if(_new_size<=buf_size){
        size=_new_size;
        return true;
    }
    // generate aligned memory allocations
    
    buf_size = _new_size + ((_new_size%4)?(4 - (_new_size%4)):0);
    
    size = _new_size;
    value_buffer = (void*)realloc(value_buffer, buf_size );
    if((result = (value_buffer != NULL))) {
        if(clear_mem) {
            //in case of string we reset the intere memory space
            std::memset(value_buffer, 0, buf_size);
        }
    } else {
        AVLERR_ << "## cannot set size: "<<_new_size;
        
    }
    return result;
}


/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeValue::markAsChanged() {
    sharedBitmapChangedAttribute->set(index);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeValue::markAsUnchanged() {
    sharedBitmapChangedAttribute->reset(index);
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::isGood() {
    return value_buffer!= NULL;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
CDataWrapper *AttributeValue::getValueAsCDatawrapperPtr(bool from_json) {
    CDataWrapper *result = NULL;
    if(!from_json) {
        result = new CDataWrapper((const char *)value_buffer);
    } else {
        result = new CDataWrapper();
        result->setSerializedJsonData((const char *)value_buffer);
    }
    return result;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeValue::writeToCDataWrapper(CDataWrapper& data_wrapper) {
    switch(type) {
        case chaos::DataType::TYPE_BYTEARRAY:{
            data_wrapper.addBinaryValue(name, (const char *)value_buffer, size);
            break;
        }
        case chaos::DataType::TYPE_STRING:{
            data_wrapper.addStringValue(name, std::string((const char *)value_buffer, size));
            break;
        }
            
        case chaos::DataType::TYPE_BOOLEAN:{
            data_wrapper.addBoolValue(name, *getValuePtr<bool>());
            break;
        }
            
        case chaos::DataType::TYPE_DOUBLE:{
            data_wrapper.addDoubleValue(name, *getValuePtr<double>());
            break;
        }
            
        case chaos::DataType::TYPE_INT32:{
            data_wrapper.addInt32Value(name, *getValuePtr<int32_t>());
            break;
        }
            
        case chaos::DataType::TYPE_INT64:{
            data_wrapper.addInt64Value(name, *getValuePtr<int64_t>());
            break;
        }
        default:
            break;
    }
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
std::string AttributeValue::toString() {
    switch(type) {
        case chaos::DataType::TYPE_BYTEARRAY:{
            return "binary_data";
        }
        case chaos::DataType::TYPE_STRING:{
            return std::string((const char *)value_buffer, size);
        }
            
        case chaos::DataType::TYPE_BOOLEAN:{
            return boost::lexical_cast<std::string>(*getValuePtr<bool>());
        }
            
        case chaos::DataType::TYPE_DOUBLE:{
            return boost::lexical_cast<std::string>(*getValuePtr<double>());
        }
            
        case chaos::DataType::TYPE_INT32:{
            return boost::lexical_cast<std::string>(*getValuePtr<int32_t>());
        }
            
        case chaos::DataType::TYPE_INT64:{
            return boost::lexical_cast<std::string>(*getValuePtr<int64_t>());
        }
            
        default:
            break;
    }
    return "bad type";
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
std::string AttributeValue::toString(int double_precision) {
    switch(type) {
        case chaos::DataType::TYPE_BYTEARRAY:{
            return "binary_data";
        }
        case chaos::DataType::TYPE_STRING:{
            return std::string((const char *)value_buffer, size);
        }
            
        case chaos::DataType::TYPE_BOOLEAN:{
            return boost::lexical_cast<std::string>(*getValuePtr<bool>());
        }
            
        case chaos::DataType::TYPE_DOUBLE:{
            stringstream f;
            f << "%." <<double_precision<< "f";
            return CHAOS_FORMAT(f.str(), %*getValuePtr<double>());
        }
            
        case chaos::DataType::TYPE_INT32:{
            return boost::lexical_cast<std::string>(*getValuePtr<int32_t>());
        }
            
        case chaos::DataType::TYPE_INT64:{
            return boost::lexical_cast<std::string>(*getValuePtr<int64_t>());
        }
            
        default:
            break;
    }
    return "bad type";
}

//!return value as CDataVariant
CDataVariant AttributeValue::getAsVariant() {
    return CDataVariant(type,
                        (const char *)value_buffer,
                        size);
}
