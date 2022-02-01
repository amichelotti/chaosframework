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
                               chaos::DataType::DataType _type,
                               const std::vector<chaos::DataType::BinarySubtype>& _sub_type):
value_buffer(NULL),
size(_size),
name(_name),
index(_index),
buf_size(0),
type(_type),
sub_type(_sub_type),
sharedBitmapChangedAttribute(NULL){
    if(_type == DataType::TYPE_STRING) {
        size += 1;
    }
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
        value_buffer=NULL;
    }
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
    

      switch (type) {
        case DataType::TYPE_BOOLEAN: {
            bool bv = *(bool*)value_ptr;
            //copy string to buffer
             if(old_value.bdata!=bv){
                *(bool*)value_buffer=bv;
                tag_has_changed =true;
                old_value.bdata=bv;
            } else {
                tag_has_changed =false;

            }
         
            break;
        }
        case DataType::TYPE_INT32: {
            int32_t i32v = *(int32_t*)value_ptr;
            if(old_value.i32data!=i32v){
                *(int32_t*)value_buffer=i32v;
                tag_has_changed =true;
                old_value.i32data=i32v;
            }else {
                tag_has_changed =false;

            }
            //copy string to buffer
         
            break;
        }
        case DataType::TYPE_INT64: {
            int64_t i64v = *(int64_t*)value_ptr;
             if(old_value.i64data!=i64v){
                *(int64_t*)value_buffer=i64v;
                tag_has_changed =true;
                old_value.i64data=i64v;
            }else {
                tag_has_changed =false;

            }
            //copy string to buffer
          
            break;
        }
        case DataType::TYPE_UINT64: {
            uint64_t i64v = *(uint64_t*)value_ptr;
             if(old_value.i64data!=i64v){
                *(uint64_t*)value_buffer=i64v;
                tag_has_changed =true;
                old_value.i64data=i64v;
            }else {
                tag_has_changed =false;

            }
            //copy string to buffer
          
            break;
        }
        case DataType::TYPE_DOUBLE: {
            double dv = *(double*)value_ptr;
             if(old_value.ddata!=dv){
                *(double*)value_buffer=dv;
                tag_has_changed =true;
                old_value.ddata=dv;
            }else {
                tag_has_changed =false;

            }
           
            break;
        }
        case DataType::TYPE_CLUSTER:

        case DataType::TYPE_STRING: {
            const std::string value((const char*)value_ptr,size);
            if(old_string!=value){
                if(!grow((uint32_t)value.size()+1)) return false;
                //copy string to buffer
                char*ptr=(char*)value_buffer;
                memcpy((void*)ptr,value.c_str(),value.size());
                *(ptr+value.size())=0;
                tag_has_changed =true;
                old_string=value;

            } else {
                tag_has_changed =false;

            }
            break;
        }
            
        
        default:
            std::memcpy(value_buffer, value_ptr, value_size);

            break;
    }
    
  
    //copy the new value
    std::memcpy(value_buffer, value_ptr, value_size);
    
    //set the relative field for set has changed
    if(tag_has_changed) sharedBitmapChangedAttribute->set(index);
    return true;
}

bool AttributeValue::setStringValue(const std::string& value,
                                    bool tag_has_changed,
                                    bool enlarge_memory) {
    if(enlarge_memory == true &&
       !grow((uint32_t)value.size()+1)) return false;
    
    if(value.size()==0) {
        std::memset(static_cast<char*>(value_buffer),
                    0,
                    size);
        return true;
    }
    CHAOS_ASSERT(value_buffer)
    std::strncpy(static_cast<char*>(value_buffer),
                 value.c_str(),
                 size);
   // AVLDBG_<<" SET STRING VALUE:"<<static_cast<char*>(value_buffer)<<" size:"<<size<< " original size:"<<value.size();
    return true;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::setValue(CDataWrapper& attribute_value,
                              bool tag_has_changed) {
  //  AVLDBG_<<" SET VALUE:"<<attribute_value.getJSONString();
	attribute_value.copyAllTo(cdvalue);
    setStringValue(attribute_value.getCompliantJSONString(),tag_has_changed,true);
	//set the relative field for set has changed
	if(tag_has_changed) sharedBitmapChangedAttribute->set(index);
	 return true;

}
bool AttributeValue::setValue(const CDataVariant& attribute_value,
                              bool tag_has_changed) {
    //if(type != attribute_value.getType()) return false;
    switch (attribute_value.getType()) {
        case DataType::TYPE_BOOLEAN: {
             if(!grow(sizeof(bool))) return false;
            bool bv = attribute_value.asBool();
            old_value.bdata=*(bool*)value_buffer;
            *(bool*)value_buffer=bv;

            //copy string to buffer
             if(old_value.bdata!=bv){
                tag_has_changed =true;
            } else {
                tag_has_changed =false;

            }
          /*  std::memcpy(value_buffer,
                        &bv,
                        sizeof(bool));*/
            break;
        }
        case DataType::TYPE_INT32: {
            if(!grow(sizeof(int32_t))) return false;
            int32_t i32v = attribute_value.asInt32();
             old_value.i32data=*(int32_t*)value_buffer;

            *(int32_t*)value_buffer=i32v;

            if(old_value.i32data!=i32v){
                tag_has_changed =true;
            }else {
                tag_has_changed =false;

            }
            //copy string to buffer
           /* 
           std::memcpy(value_buffer,
                        &i32v,
                        sizeof(int32_t));
            */
            break;
        }
        case DataType::TYPE_INT64: {
            if(!grow(sizeof(int64_t))) return false;
            int64_t i64v = attribute_value.asInt64();
            old_value.i64data=*(int64_t*)value_buffer;

            *(int64_t*)value_buffer=i64v;

             if(old_value.i64data!=i64v){
                tag_has_changed =true;
            }else {
                tag_has_changed =false;

            }
            //copy string to buffer
            /*std::memcpy(value_buffer,
                        &i64v,
                        sizeof(int64_t));*/
            break;
        }
        case DataType::TYPE_UINT64: {
            if(!grow(sizeof(int64_t))) return false;
            uint64_t i64v = attribute_value.asUInt64();
            old_value.i64data=*(uint64_t*)value_buffer;

            *(uint64_t*)value_buffer=i64v;

             if(old_value.i64data!=i64v){
                tag_has_changed =true;
            }else {
                tag_has_changed =false;

            }
            //copy string to buffer
            /*std::memcpy(value_buffer,
                        &i64v,
                        sizeof(int64_t));*/
            break;
        }
        case DataType::TYPE_DOUBLE: {
            if(!grow(sizeof(double))) return false;
            double dv = attribute_value.asDouble();
            old_value.ddata=*(double*)value_buffer;
            *(double*)value_buffer=dv;

             if(old_value.ddata!=dv){
                tag_has_changed =true;
            }else {
                tag_has_changed =false;

            }
            //copy string to buffer
           /* std::memcpy(value_buffer,
                        &dv,
                        sizeof(double));*/
            break;
        }
        case DataType::TYPE_CLUSTER:

        case DataType::TYPE_STRING: {
            const std::string value = attribute_value.asString();
            old_string=(char*)value_buffer;
            char*ptr=(char*)value_buffer;

            memcpy((void*)ptr,value.c_str(),value.size());
            *(ptr+value.size())=0;

            if(old_string!=value){
                if(!grow((uint32_t)value.size()+1)) return false;
                //copy string to buffer
                tag_has_changed =true;

            } else {
                tag_has_changed =false;

            }
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
bool AttributeValue::grow(uint32_t value_size) {
    if(setNewSize(value_size,false)==false){
        return false;
    }
    return true;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
bool AttributeValue::setNewSize(uint32_t _new_size,
                                bool clear_mem) {
    bool result = true;
    if(_new_size<=buf_size){
        size =_new_size;
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
CDWUniquePtr AttributeValue::getValueAsCDatawrapperPtr(bool from_json) {
    CDWUniquePtr result;
    if(!from_json) {
        result.reset(new CDataWrapper((const char *)value_buffer));
    } else {
        result.reset(new CDataWrapper());
        result->setSerializedJsonData((const char *)value_buffer);
    }
    return result;
}

/*---------------------------------------------------------------------------------
 
 ---------------------------------------------------------------------------------*/
void AttributeValue::writeToCDataWrapper( CDataWrapper& data_wrapper) {
    switch(type) {
        case chaos::DataType::TYPE_BYTEARRAY:{
            switch(sub_type.size()) {
                case 1:
                    data_wrapper.addBinaryValue(name, sub_type[0], (const char *)value_buffer, size);
                    break;
                    
                default:
                    data_wrapper.addBinaryValue(name, (const char *)value_buffer, size);
                    break;
            }
            break;
        }
        case chaos::DataType::TYPE_CLUSTER:{
            CDataWrapper p;
            if(value_buffer && ((*(const char *)value_buffer)!=0)){
                p.setSerializedJsonData((const char*)value_buffer);
            }
            data_wrapper.addCSDataValue(name,p);

            
            break;
        }
        case chaos::DataType::TYPE_STRING:{
            unsigned long str_len = value_buffer?std::strlen((const char *)value_buffer):0;
            str_len = (str_len>=size?size:str_len);
            data_wrapper.addStringValue(name, (str_len?std::string(static_cast<const char*>(value_buffer), str_len):""));
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
        case chaos::DataType::TYPE_UINT64:{
            data_wrapper.addUInt64Value(name, *getValuePtr<uint64_t>());
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
        case chaos::DataType::TYPE_CLUSTER:
        case chaos::DataType::TYPE_STRING:{
            return std::string((const char *)value_buffer, size);
        }
            
        case chaos::DataType::TYPE_BOOLEAN:{
            return ChaosToString(*getValuePtr<bool>());
        }
            
        case chaos::DataType::TYPE_DOUBLE:{
            return ChaosToString(*getValuePtr<double>());
        }
            
        case chaos::DataType::TYPE_INT32:{
            return ChaosToString(*getValuePtr<int32_t>());
        }
            
        case chaos::DataType::TYPE_INT64:{
            return ChaosToString(*getValuePtr<int64_t>());
        }
        case chaos::DataType::TYPE_UINT64:{
            return ChaosToString(*getValuePtr<uint64_t>());
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
        
        case chaos::DataType::TYPE_CLUSTER:
        case chaos::DataType::TYPE_STRING:{
            return std::string((const char *)value_buffer, size);
        }
            
        case chaos::DataType::TYPE_BOOLEAN:{
            return ChaosToString(*getValuePtr<bool>());
        }
            
        case chaos::DataType::TYPE_DOUBLE:{
            stringstream f;
            f << "%." <<double_precision<< "f";
            return CHAOS_FORMAT(f.str(), %*getValuePtr<double>());
        }
            
        case chaos::DataType::TYPE_INT32:{
            return ChaosToString(*getValuePtr<int32_t>());
        }
            
        case chaos::DataType::TYPE_INT64:{
            return ChaosToString(*getValuePtr<int64_t>());
        }
         case chaos::DataType::TYPE_UINT64:{
            return ChaosToString(*getValuePtr<uint64_t>());
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
