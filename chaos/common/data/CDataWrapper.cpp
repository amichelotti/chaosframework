/*
 *	CDataWrapper.cpp
 *	!CHAOS
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
#include <chaos/common/chaos_constants.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/utility/Base64Util.h>
#include <boost/lexical_cast.hpp>

using namespace chaos;
using namespace chaos::common::data;

using namespace chaos::common::utility;
#pragma mark Utility
#define ADD_VECTOR(v,ctype,bsontype){\
for( std::vector<ctype>::const_iterator i=v.begin();i!=v.end();i++){\
append ##bsontype ##ToArray(*i);}}

#define ITER_TYPE(i) ((bson_type_t) * ((i)->raw + (i)->type))

#define ALLOCATE_BSONT(x) ChaosBsonShrdPtr(x, &bsonDeallocator)

#define ACCESS_BSON(x) static_cast<bson_t*>(x.get())
#define CW_CAST_EXCEPTION(type){\
std::stringstream ss;\
ss<<"cannot get or cast to '" #type  "' '"<<key<<"' ds:" << getJSONString();\
throw CException(1, ss.str(), __PRETTY_FUNCTION__);}

#define ENSURE_ARRAY(x) \
if(x.get() == NULL) {array_index = 0; x = ALLOCATE_BSONT(bson_new());}

#define FIND_AND_CHECK(k,c)\
bson_iter_t element_found;\
bson_iter_init(&element_found, ACCESS_BSON(bson));\
if(bson_iter_find_case(&element_found, key.c_str()) && c(&element_found))

#define INIT_ITERATOR(key) \
bool keyfound;\
bson_iter_t element_found;\
bson_iter_init(&element_found, ACCESS_BSON(bson));\
keyfound=bson_iter_find_case(&element_found, key.c_str());

#define GET_VALUE(t,c) \
if(keyfound&& (c(&element_found))){return bson_iter_##t (&element_found);}

#define GET_64UVALUE() \
if(keyfound&& (BSON_ITER_HOLDS_TIMESTAMP(&element_found))){uint64_t dst;uint32_t*u=(uint32_t*)&dst;bson_iter_timestamp(&element_found,&u[1],&u[0]);return dst ;}

#define GET_VALUE_AS_STRING(t,c) \
if(keyfound&& (c(&element_found))){std::stringstream ss;ss<<bson_iter_##t (&element_found);return ss.str();}

#define GET_TIMESTAMP_AS_STRING() \
if(keyfound&& (BSON_ITER_HOLDS_TIMESTAMP(&element_found))){uint64_t dst;uint32_t*u=(uint32_t*)&dst;bson_iter_timestamp(&element_found,&u[1],&u[0]);std::stringstream ss;ss<<dst;return ss.str();}

static void bsonDeallocator(bson_t* bson) {if(bson){bson_destroy(bson);}}

static void bsonValueDestroy(bson_value_t* bson_values) {if(bson_values){bson_value_destroy(bson_values);}}

#pragma mark CDataWrapper
CDataWrapper::CDataWrapper():
bson(ALLOCATE_BSONT(bson_new())),
array_index(0){
        if(bson==NULL){
              throw CException(-53, "Invalid BSON", __PRETTY_FUNCTION__);

    }
}
bool CDataWrapper::isJSON(const::std::string&str){
    if(str.size()){
        try{
            CDataWrapper cd;
            cd.setSerializedJsonData(str.c_str());
            return true;
        } catch(...){

        }
    }
    return false;
}

CDataWrapper::CDataWrapper(const bson_t *copy_bson):
array_index(0){
    if(copy_bson != NULL) {
        bson = ALLOCATE_BSONT(bson_copy(copy_bson));
    } else {
        bson = ALLOCATE_BSONT(bson_new());
    }
    if(bson==NULL){
              throw CException(-61, "Invalid BSON", __PRETTY_FUNCTION__);

    }}
bool CDataWrapper::operator==(const CDataWrapper&d) const {
    int32_t siz1,siz2;
    const char*buf1=getBSONRawData(siz1);
    const char*buf2=d.getBSONRawData(siz2);
    if(siz1!=siz2){
        return false;
    }
    return (memcmp(buf1,buf2,siz1)==0);

}

CDataWrapper::CDataWrapper(const char* mem_ser,
                           uint32_t mem_size):
array_index(0){
    if(mem_ser != NULL || mem_size) {
        bson = ALLOCATE_BSONT(bson_new_from_data((const uint8_t*)mem_ser,
                                                 mem_size));
    } else {
        bson = ALLOCATE_BSONT(bson_new());
    }
    if(bson==NULL){
              throw CException(-51, "Invalid BSON", __PRETTY_FUNCTION__);

    }
}

CDataWrapper::CDataWrapper(const char* mem_ser):
array_index(0) {
    if(mem_ser) {
        uint32_t size = BSON_UINT32_FROM_LE(*reinterpret_cast<const uint32_t *>(mem_ser));
        bson = ALLOCATE_BSONT(bson_new_from_data((const uint8_t*)mem_ser,
                                                 size));
    } else {
        bson = ALLOCATE_BSONT(bson_new());
    }
 if(bson==NULL){
              throw CException(-52, "Invalid BSON ", __PRETTY_FUNCTION__);

    }
}

CDataWrapper::CDataWrapper(const std::string& json_document):
array_index(0) {
    bson_error_t err;
    bson = ALLOCATE_BSONT(bson_new_from_json((const uint8_t*)json_document.c_str(),
                                             json_document.size(),
                                             &err));
    if(!bson.get()) {
        bson = ALLOCATE_BSONT(bson_new());
    }
}

CDataWrapper::~CDataWrapper(){}

ChaosUniquePtr<CDataWrapper> CDataWrapper::instanceFromJson(const std::string& json_serialization) {
    return ChaosUniquePtr<CDataWrapper>(new CDataWrapper(json_serialization));
}

ChaosUniquePtr<CDataWrapper> CDataWrapper::clone() const{
  /*  CDataWrapper*ptr=new CDataWrapper();
    copyAllTo(*ptr);
    return ChaosUniquePtr<CDataWrapper>(ptr);*/
    return ChaosUniquePtr<CDataWrapper>(new CDataWrapper(bson.get()));
}


//add a csdata value
void CDataWrapper::addCSDataValue(const std::string& key,
                                  const CDataWrapper& sub_object) {
    bson_append_document(ACCESS_BSON(bson),
                         key.c_str(),
                         (int)key.size(),
                         ACCESS_BSON(sub_object.bson));
}
int CDataWrapper::countKeys() const{
    return bson_count_keys(ACCESS_BSON(bson));
}

//add a string value
void CDataWrapper::addStringValue(const std::string& key, const string& value,const int max_size) {
      
        bson_append_utf8(ACCESS_BSON(bson),
                        key.c_str(),
                        (int)key.size(),
                        value.c_str(),
                        value.size());
    
}

//append a strin gto an open array
void CDataWrapper::appendStringToArray(const string& value) {
    ENSURE_ARRAY(bson_tmp_array);
    bson_append_utf8(ACCESS_BSON(bson_tmp_array),
                     ChaosToString(array_index++).c_str(),
                     -1,
                     value.c_str(),
                     (int)value.size());
}
void CDataWrapper::appendBooleanToArray(bool value){
    ENSURE_ARRAY(bson_tmp_array);
    bson_append_bool(ACCESS_BSON(bson_tmp_array),
                     ChaosToString(array_index++).c_str(),
                     -1,
                     value);
}

//append a strin gto an open array
void CDataWrapper::appendInt32ToArray(int32_t value) {
    ENSURE_ARRAY(bson_tmp_array);
    bson_append_int32(ACCESS_BSON(bson_tmp_array),
                      ChaosToString(array_index++).c_str(),
                      -1,
                      value);
}

//append a strin gto an open array
void CDataWrapper::appendInt64ToArray(int64_t value) {
    ENSURE_ARRAY(bson_tmp_array);
    bson_append_int64(ACCESS_BSON(bson_tmp_array),
                      ChaosToString(array_index++).c_str(),
                      -1,
                      value);
}

//append a strin gto an open array
void CDataWrapper::appendUInt64ToArray(uint64_t value) {
    ENSURE_ARRAY(bson_tmp_array);
    bson_append_timestamp(ACCESS_BSON(bson_tmp_array),
                      ChaosToString(array_index++).c_str(),
                      -1,
                      value>>32,
                      value&0xFFFFFFFF);
}

//append a strin gto an open array
void CDataWrapper::appendDoubleToArray(double value) {
    ENSURE_ARRAY(bson_tmp_array);
    bson_append_double(ACCESS_BSON(bson_tmp_array),
                       ChaosToString(array_index++).c_str(),
                       -1,
                       value);
}

//appen a CDataWrapper to an open array
void CDataWrapper::appendCDataWrapperToArray(const CDataWrapper& value) {
    ENSURE_ARRAY(bson_tmp_array);
    bson_append_document(ACCESS_BSON(bson_tmp_array),
                         ChaosToString(array_index++).c_str(),
                         -1,
                         value.bson.get());
}
void CDataWrapper::append(const std::string&key,CMultiTypeDataArrayWrapperSPtr&val) {
bson_append_array(ACCESS_BSON(bson),
                      key.c_str(),
                      (int)key.size(),
                      val->array_doc);
}


//finalize the array into a key for the current dataobject
void CDataWrapper::finalizeArrayForKey(const std::string& key) {
    ENSURE_ARRAY(bson_tmp_array);

    FIND_AND_CHECK(key, BSON_ITER_HOLDS_ARRAY){
        // if already exists
        CMultiTypeDataArrayWrapperSPtr v=getVectorValue(key);
        removeKey(key);
        array_index=0;
        bson_t *tmp=bson_new();
        for(int cnt=0;cnt<v->size();cnt++){
            bson_value_t * bv=v->getBSONElementAtIndex(cnt);
            if(bv){
                std::string k=ChaosToString(array_index++);
                bson_append_value(tmp,k.c_str(),-1,bv);
            }
             
        }
        bson_iter_t it;
        bson_iter_init(&it, ACCESS_BSON(bson_tmp_array));
        while(bson_iter_next(&it)) {
            const bson_value_t * bv= bson_iter_value(&it);
            std::string k=ChaosToString(array_index++);
            bson_append_value(tmp,k.c_str(),-1,bv);
        }          

        bson_append_array(ACCESS_BSON(bson),
                      key.c_str(),
                      (int)key.size(),
                      tmp);
        bson_tmp_array.reset();
        bson_free(tmp);
        return;
    } 
    bson_append_array(ACCESS_BSON(bson),
                      key.c_str(),
                      (int)key.size(),
                      bson_tmp_array.get());
    bson_tmp_array.reset();
}

//return a vectorvalue for a key
CMultiTypeDataArrayWrapperSPtr CDataWrapper::getVectorValue(const std::string& key)  const{
    return CMultiTypeDataArrayWrapperSPtr(new CMultiTypeDataArrayWrapper(bson, key));
}

void CDataWrapper::addNullValue(const std::string& key) {
    bson_append_null(ACCESS_BSON(bson),
                     key.c_str(),
                     (int)key.size());
}
//add a long value
void CDataWrapper::addInt32Value(const std::string& key, int32_t value) {
    bson_append_int32(ACCESS_BSON(bson),
                      key.c_str(),
                      (int)key.size(),
                      value);
}
//add a long value
void CDataWrapper::addUInt32Value(const std::string& key, uint32_t value) {
    bson_append_int32(ACCESS_BSON(bson),
                      key.c_str(),
                      (int)key.size(),
                      static_cast<int32_t>(value));
}
//add a double value
void CDataWrapper::addDoubleValue(const std::string& key, double value) {
    bson_append_double(ACCESS_BSON(bson),
                       key.c_str(),
                       (int)key.size(),
                       value);
}

//add a integer value
void CDataWrapper::addInt64Value(const std::string& key, int64_t value) {
    bson_append_int64(ACCESS_BSON(bson),
                      key.c_str(),
                      (int)key.size(),
                      value);
}

//add a integer value
void CDataWrapper::addUInt64Value(const std::string& key, uint64_t value) {
    bson_append_timestamp(ACCESS_BSON(bson),
                      key.c_str(),
                      (int)key.size(),
                      value>>32,
                      value&0xFFFFFFFF);
}

//get a csdata value
CDWUniquePtr CDataWrapper::getCSDataValue(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_DOCUMENT){
        uint32_t document_len = 0;
        const uint8_t *document = NULL;
        bson_iter_document(&element_found,
                           &document_len,
                           &document);
        return CDWUniquePtr(new CDataWrapper((const char *)document,document_len));
    } else {
        return CDWUniquePtr(new CDataWrapper());
    }
}
void CDataWrapper::getCSDataValue(const std::string& key,chaos::common::data::CDataWrapper&cd) const{
   CDWUniquePtr res=getCSDataValue(key);
   if(res.get()){
       res->copyAllTo(cd);
   }
}

ChaosUniquePtr<chaos::common::data::CDataWrapper> CDataWrapper::getCSProjection(const std::vector<std::string>&keys) const{
    CDWUniquePtr ret(new CDataWrapper());
    for(std::vector<std::string>::const_iterator i=keys.begin();i!=keys.end();i++){
        copyKeyTo(*i,*ret.get());
    }
    return ret;

}

std::string CDataWrapper::getJsonValue(const std::string& key) const{
    ChaosUniquePtr<CDataWrapper> tmp=getCSDataValue(key);
    if(tmp.get()){
        return tmp->getJSONString();
    }
    return std::string();
}

//get string value
string  CDataWrapper::getStringValue(const std::string& key) const{
    {
        FIND_AND_CHECK(key, BSON_ITER_HOLDS_UTF8){
        return std::string(bson_iter_utf8(&element_found, NULL));
    }
    }
    INIT_ITERATOR(key);

    GET_VALUE_AS_STRING(int32,BSON_ITER_HOLDS_INT32);
    GET_VALUE_AS_STRING(double,BSON_ITER_HOLDS_DOUBLE);
    GET_VALUE_AS_STRING(int64,BSON_ITER_HOLDS_INT64);
    GET_TIMESTAMP_AS_STRING();

    GET_VALUE_AS_STRING(bool,BSON_ITER_HOLDS_BOOL);
    GET_VALUE_AS_STRING(date_time,BSON_ITER_HOLDS_DATE_TIME);


    return std::string();
}

//get string value
const char *  CDataWrapper::getCStringValue(const std::string& key) const{
    return getRawValuePtr(key);
}

//add a integer value
int32_t CDataWrapper::getInt32Value(const std::string& key) const{
    INIT_ITERATOR(key);
    GET_VALUE(int32,BSON_ITER_HOLDS_INT32);
    GET_VALUE(bool,BSON_ITER_HOLDS_BOOL);
    GET_VALUE(double,BSON_ITER_HOLDS_DOUBLE);
 if(isStringValue(key)){
        return atoi(getStringValue(key).c_str());
    }
    CW_CAST_EXCEPTION(int32);
}
//add a integer value
uint32_t CDataWrapper::getUInt32Value(const std::string& key) const{
    INIT_ITERATOR(key);
    GET_VALUE(int32,BSON_ITER_HOLDS_INT32);
    GET_VALUE(bool,BSON_ITER_HOLDS_BOOL);
    if(isStringValue(key)){
        return atoi(getStringValue(key).c_str());
    }
    CW_CAST_EXCEPTION(uint32);
}
//add a integer value
int64_t CDataWrapper::getInt64Value(const std::string& key) const{
    INIT_ITERATOR(key);
    GET_VALUE(int64,BSON_ITER_HOLDS_INT64);
    
    GET_64UVALUE();
    GET_VALUE(date_time,BSON_ITER_HOLDS_DATE_TIME);
    GET_VALUE(int32,BSON_ITER_HOLDS_INT32);
    GET_VALUE(double,BSON_ITER_HOLDS_DOUBLE);
    GET_VALUE(bool,BSON_ITER_HOLDS_BOOL);
    if(isStringValue(key)){
        return atoll(getStringValue(key).c_str());
    }
    CW_CAST_EXCEPTION(int64);
}
//add a integer value
uint64_t CDataWrapper::getUInt64Value(const std::string& key) const{
    INIT_ITERATOR(key);
    GET_64UVALUE();
    GET_VALUE(int64,BSON_ITER_HOLDS_INT64);
    GET_VALUE(date_time,BSON_ITER_HOLDS_DATE_TIME);
    GET_VALUE(int32,BSON_ITER_HOLDS_INT32);
    GET_VALUE(double,BSON_ITER_HOLDS_DOUBLE);
    GET_VALUE(bool,BSON_ITER_HOLDS_BOOL);
     if(isStringValue(key)){
        return atoll(getStringValue(key).c_str());
    }
    CW_CAST_EXCEPTION(uint64);
}
//add a integer value
double CDataWrapper::getDoubleValue(const std::string& key) const{
    INIT_ITERATOR(key);
    GET_VALUE(double,BSON_ITER_HOLDS_DOUBLE);
    GET_VALUE(int64,BSON_ITER_HOLDS_INT64);
    GET_64UVALUE();
    GET_VALUE(int32,BSON_ITER_HOLDS_INT32);
    GET_VALUE(bool,BSON_ITER_HOLDS_BOOL);
    if(isStringValue(key)){
        return atof(getStringValue(key).c_str());
    }
    CW_CAST_EXCEPTION(double);
}

//get a bool value
bool  CDataWrapper::getBoolValue(const std::string& key) const{
    INIT_ITERATOR(key);
    GET_VALUE(bool,BSON_ITER_HOLDS_BOOL);
    GET_VALUE(int32,BSON_ITER_HOLDS_INT32);
     if(isStringValue(key)){
        return (bool)atoi(getStringValue(key).c_str());
    }
    CW_CAST_EXCEPTION(bool);
}


//set a binary data value
void CDataWrapper::addBinaryValue(const std::string& key,
                                  const char *buff,
                                  int bufLen) {
    bson_append_binary(ACCESS_BSON(bson),
                       key.c_str(),
                       (int)key.size(),
                       BSON_SUBTYPE_BINARY,
                       (const uint8_t *)buff,
                       bufLen);
}

chaos::DataType::BinarySubtype CDataWrapper::getBinarySubtype(const std::string& key) const{
    bson_subtype_t subtype = BSON_SUBTYPE_USER;
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_BINARY){
        const uint8_t *buff = NULL;
        uint32_t buf_len;
        bson_iter_binary(&element_found,
                         &subtype,
                         &buf_len,
                         &buff);
    }
    return (chaos::DataType::BinarySubtype)(subtype-BSON_SUBTYPE_USER);
}

void CDataWrapper::addBinaryValue(const std::string& key,
                                  chaos::DataType::BinarySubtype sub_type,
                                  const char *buff,
                                  int buf_len) {
    bson_append_binary(ACCESS_BSON(bson),
                       key.c_str(),
                       (int)key.size(),
                       (bson_subtype_t)(sub_type+BSON_SUBTYPE_USER),
                       (const uint8_t *)buff,
                       buf_len);
}
void CDataWrapper::appendArray(const std::string& key,char* arr,int count){
    addBinaryValue(key,chaos::DataType::SUB_TYPE_INT8,(const char*)arr,count);
}
void CDataWrapper::appendArray(const std::string& key,int32_t* arr,int count){
    addBinaryValue(key,chaos::DataType::SUB_TYPE_INT32,(const char*)arr,count*sizeof(int32_t));
}
void CDataWrapper::appendArray(const std::string& key,uint32_t* arr,int count){
    addBinaryValue(key,chaos::DataType::SUB_TYPE_UINT32,(const char*)arr,count*sizeof(int32_t));
}
void CDataWrapper::appendArray(const std::string& key,double* arr,int count){
    addBinaryValue(key,chaos::DataType::SUB_TYPE_DOUBLE,(const char*)arr,count*sizeof(double));
}
void CDataWrapper::appendArray(const std::string& key,float* arr,int count){
    addBinaryValue(key,chaos::DataType::SUB_TYPE_FLOAT,(const char*)arr,count*sizeof(float));
}
void CDataWrapper::appendArray(const std::string& key,int16_t* arr,int count){
     addBinaryValue(key,chaos::DataType::SUB_TYPE_INT16,(const char*)arr,count*sizeof(int16_t));
}
void CDataWrapper::appendArray(const std::string& key,uint16_t* arr,int count){
     addBinaryValue(key,chaos::DataType::SUB_TYPE_UINT16,(const char*)arr,count*sizeof(int16_t));
}
void CDataWrapper::appendArray(const std::string& key,int8_t* arr,int count){
     addBinaryValue(key,chaos::DataType::SUB_TYPE_INT8,(const char*)arr,count*sizeof(int16_t));
}
void CDataWrapper::appendArray(const std::string& key,uint8_t* arr,int count){
     addBinaryValue(key,chaos::DataType::SUB_TYPE_UINT8,(const char*)arr,count*sizeof(int16_t));
}
void CDataWrapper::appendArray(const std::string& key,int64_t* arr,int count){
    addBinaryValue(key,chaos::DataType::SUB_TYPE_INT64,(const char*)arr,count*sizeof(int64_t));
}
void CDataWrapper::appendArray(const std::string& key,uint64_t* arr,int count){
    addBinaryValue(key,chaos::DataType::SUB_TYPE_UINT64,(const char*)arr,count*sizeof(uint64_t));
}

void CDataWrapper::append(const std::string& key,int32_t val){
    addInt32Value(key, val);
}

void CDataWrapper::append(const std::string& key,int64_t val){
    addInt64Value(key, val);
}

void CDataWrapper::append(const std::string& key,double val){
    addDoubleValue(key, val);
}

void CDataWrapper::append(const std::string& key,bool val){
    addBoolValue(key, val);
}

void CDataWrapper::append(const std::string& key,const std::string& val){
    addStringValue(key, val);
}
void CDataWrapper::append(const std::string& key,const char* val){
    addStringValue(key, val);
}
void CDataWrapper::append(const std::string& key,const CDataWrapper& val){
    addCSDataValue(key,val);
}
/*
void CDataWrapper::append(const std::string& key,const std::vector<int32_t>& val){
    //ADD_VECTOR(val,int32_t,Int32);
    //finalizeArrayForKey(key);
    appendArray(key,(int32_t*)&val[0],val.size());
}

void CDataWrapper::append(const std::string& key,const std::vector<uint32_t>& val){
    //ADD_VECTOR(val,int32_t,Int32);
    //finalizeArrayForKey(key);
    appendArray(key,(uint32_t*)&val[0],val.size());
}

void CDataWrapper::append(const std::string& key,const std::vector<int64_t>& val){
    //ADD_VECTOR(val,int64_t,Int64);
    //finalizeArrayForKey(key);
    appendArray(key,(int64_t*)&val[0],val.size());

}

void CDataWrapper::append(const std::string& key,const std::vector<uint64_t>& val){
    //ADD_VECTOR(val,int64_t,Int64);
    //finalizeArrayForKey(key);
    appendArray(key,(uint64_t*)&val[0],val.size());

}

void CDataWrapper::append(const std::string& key,const std::vector<int16_t>& val){
    appendArray(key,(int16_t*)&val[0],val.size());

}

void CDataWrapper::append(const std::string& key,const std::vector<uint16_t>& val){
    appendArray(key,(uint16_t*)&val[0],val.size());

}

void CDataWrapper::append(const std::string& key,const std::vector<int8_t>& val){
    appendArray(key,(int8_t*)&val[0],val.size());

}

void CDataWrapper::append(const std::string& key,const std::vector<uint8_t>& val){
    appendArray(key,(uint8_t*)&val[0],val.size());

}

void CDataWrapper::append(const std::string& key,const std::vector<double>& val){
    //ADD_VECTOR(val,double,Double);
    //finalizeArrayForKey(key);
    appendArray(key,(double*)&val[0],val.size());

}

void CDataWrapper::append(const std::string& key,const std::vector<bool>& val){
    ADD_VECTOR(val,bool,Boolean);
    finalizeArrayForKey(key);
    //appendArray(key,(bool*)&val[0],val.size());

}
*/
void CDataWrapper::append(const std::string& key,const std::vector<std::string>& val){
    ADD_VECTOR(val,std::string,String);
    finalizeArrayForKey(key);

}
void CDataWrapper::append(const std::string& key,const std::vector<CDataWrapper>& val){
    ADD_VECTOR(val,CDataWrapper,CDataWrapper);
    finalizeArrayForKey(key);
}

void CDataWrapper::append(const std::string& key,const std::vector<int32_t>& val){
    ADD_VECTOR(val,int32_t,Int32);
    finalizeArrayForKey(key);

}
void CDataWrapper::append(const std::string& key,const std::vector<int64_t>& val){
    ADD_VECTOR(val,int64_t,Int64);
    finalizeArrayForKey(key);

}
void CDataWrapper::append(const std::string& key,const std::vector<double>& val){
    ADD_VECTOR(val,double,Double);
    finalizeArrayForKey(key);
}

void CDataWrapper::append(const std::string&key,DataType::DataType typ,const char*buf,int len){
    int i;
    if(len<=0 ||(typ!=DataType::TYPE_BOOLEAN) &&(typ!=DataType::TYPE_INT32)&&(typ!=DataType::TYPE_INT64)&&(typ!=DataType::TYPE_DOUBLE) ){
        throw CException(-51, "Invalid ARRAY TYPE/SIZE", __PRETTY_FUNCTION__);

    }
   switch (typ) {
        case DataType::TYPE_BOOLEAN:
            for(i=0;i<len/sizeof(bool);i++){
                appendBooleanToArray(((bool*)buf)[i]);
            }
            break;
        case DataType::TYPE_INT32:
          for( i=0;i<len/sizeof(int32_t);i++){
                appendInt32ToArray(((int32_t*)buf)[i]);
            }
            break;
        case DataType::TYPE_INT64:
            for( i=0;i<len/sizeof(int64_t);i++){
                appendInt64ToArray(((int64_t*)buf)[i]);
            }
            break;
        case DataType::TYPE_UINT64:
            for( i=0;i<len/sizeof(uint64_t);i++){
                appendInt64ToArray(((uint64_t*)buf)[i]);
            }
            break;
        case DataType::TYPE_DOUBLE:
        case DataType::TYPE_FLOAT:
             for( i=0;i<len/sizeof(double);i++){
                appendDoubleToArray(((double*)buf)[i]);
            }
            break;
       
        default:{
            break;
        }
    } 
    finalizeArrayForKey(key); 
}

void CDataWrapper::addVariantValue(const std::string& key,
                                   const CDataVariant& variant_value) {
    //create variant using the typed data
    switch (variant_value.getType()) {
        case DataType::TYPE_BOOLEAN:
            addBoolValue(key, variant_value.asBool());
            break;
        case DataType::TYPE_INT32:
            addInt32Value(key, variant_value.asInt32());
            break;
        case DataType::TYPE_INT64:
            addInt64Value(key, variant_value.asInt64());
            break;
        case DataType::TYPE_UINT64:
            addUInt64Value(key, variant_value.asUInt64());
            break;
        case DataType::TYPE_DOUBLE:
        case DataType::TYPE_FLOAT:
            addDoubleValue(key, variant_value.asDouble());
            break;
        case DataType::TYPE_CLUSTER:{
            addJsonValue(key,variant_value.asString());
            break;
        }
        case DataType::TYPE_STRING:
            addStringValue(key, variant_value.asString());
            break;
        case DataType::TYPE_BYTEARRAY:
            addBinaryValue(key,
                           variant_value.asCDataBuffer()->getBuffer(),
                           (uint32_t)variant_value.asCDataBuffer()->getBufferSize());
            break;
        default:{
            break;
        }
    }
}

//return the binary data value
const char* CDataWrapper::getBinaryValue(const std::string& key, uint32_t& bufLen) const{
    const uint8_t* ret = NULL;
    bson_subtype_t sub = BSON_SUBTYPE_USER;
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_BINARY){
        bson_iter_binary(&element_found,
                         &sub,
                         &bufLen,
                         &ret);
        return (const char*)ret;
    }
    return NULL;
}

CDBufferUniquePtr CDataWrapper::getBinaryValueAsCDataBuffer(const std::string &key) const{
    uint32_t buf_len = 0;
    const char* buffer = getBinaryValue(key, buf_len);
    if(buffer==NULL){
     // if not a pure binary probably is a string with a base64
        FIND_AND_CHECK(key, BSON_ITER_HOLDS_UTF8){
            std::string strb(bson_iter_utf8(&element_found, NULL));
            return Base64Util::decode(strb);
        }   
    }
    return CDBufferUniquePtr(new CDataBuffer(buffer,
                                             buf_len));
}

//check if the key is present in data wrapper
bool CDataWrapper::hasKey(const std::string& key) const{
    return bson_has_field(ACCESS_BSON(bson), key.c_str());
}

bool CDataWrapper::isVector(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_ARRAY){
        return true;
    } else {
        return false;
    }
}

//return all key contained into the object
void CDataWrapper::getAllKey(ChaosStringVector& contained_key) const{
    bson_iter_t it;
    bson_iter_init(&it, ACCESS_BSON(bson));
    while(bson_iter_next(&it)) {
        contained_key.push_back(bson_iter_key(&it));
    }
}

//return all key contained into the object
void CDataWrapper::getAllKey(ChaosStringSet& contained_key) const{
    bson_iter_t it;
    bson_iter_init(&it, ACCESS_BSON(bson));
    while(bson_iter_next(&it)) {
        contained_key.insert(bson_iter_key(&it));
    }
}
ChaosStringVector CDataWrapper::getAllKey() const{
    ChaosStringVector ret;
    getAllKey(ret);
    return ret;

}

//return all key contained into the object
uint32_t CDataWrapper::getValueSize(const std::string& key) const{
    bson_iter_t it;
    bson_iter_init(&it, ACCESS_BSON(bson));
    if(bson_iter_find_case(&it, key.c_str()) == false) return 0;
    const bson_value_t *v = bson_iter_value(&it);
    switch(v->value_type) {
        case BSON_TYPE_INT64:
        case BSON_TYPE_TIMESTAMP:
            return sizeof(int64_t);
        case BSON_TYPE_INT32:
            return sizeof(int32_t);
        case BSON_TYPE_BOOL:
            return sizeof(bool);
        case BSON_TYPE_DOUBLE:
            return sizeof(double);
        case BSON_TYPE_UTF8:
            return v->value.v_utf8.len;
        case BSON_TYPE_BINARY:
            return v->value.v_binary.data_len;
        case BSON_TYPE_ARRAY:{
            uint32_t array_len = 0;
            const uint8_t *array = NULL;
            bson_iter_array(&it, &array_len, &array);
            return array_len;
        }
        case BSON_TYPE_DOCUMENT:{
            return v->value.v_doc.data_len;
        }
        default:
            return 0;
            break;
    }
    return 0;
}

//! return the raw value ptr address
const char * CDataWrapper::getRawValuePtr(const std::string& key) const{
    bson_iter_t it;
    bson_iter_init(&it, ACCESS_BSON(bson));
    if(bson_iter_find_case(&it, key.c_str()) == false) return 0;
    const bson_value_t *v = bson_iter_value(&it);
    switch(v->value_type) {
          case BSON_TYPE_TIMESTAMP:{
          
        
            return reinterpret_cast<const char*>(&v->value.v_timestamp);
          }
        case BSON_TYPE_INT64:
        
            return reinterpret_cast<const char*>(&v->value.v_int64);
        case BSON_TYPE_INT32:
            return reinterpret_cast<const char*>(&v->value.v_int32);
        case BSON_TYPE_BOOL:
            return reinterpret_cast<const char*>(&v->value.v_bool);
        case BSON_TYPE_DOUBLE:
            return reinterpret_cast<const char*>(&v->value.v_double);
        case BSON_TYPE_UTF8:
            return static_cast<const char*>(v->value.v_utf8.str);
        case BSON_TYPE_BINARY:
            return reinterpret_cast<const char*>(v->value.v_binary.data);
        case BSON_TYPE_ARRAY:{
            uint32_t array_len = 0;
            const uint8_t *array = NULL;
            bson_iter_array(&it, &array_len, &array);
            return reinterpret_cast<const char*>(array);
        }
        case BSON_TYPE_DOCUMENT:{
            return reinterpret_cast<const char*>(v->value.v_doc.data);
        }
        default:
            return NULL;
            break;
    }
}

//add a bool value
void CDataWrapper::addBoolValue(const std::string& key, bool value) {
    bson_append_bool(ACCESS_BSON(bson),
                     key.c_str(),
                     (int)key.size(),
                     value);
}

/*
 Return the Serialized buffer object taht contain the memory,
 the requester of this method shuld be deallocate the object
 */
#ifndef _WIN32
__attribute__((__deprecated__))
#else
__declspec(deprecated)
#endif
SerializationBufferUPtr CDataWrapper::getBSONData() const{
    const char * buff = reinterpret_cast<const char*>(bson_get_data(ACCESS_BSON(bson)));
    if(!buff) return SerializationBufferUPtr();
    return SerializationBufferUPtr(new SerializationBuffer(buff, bson->len));
}

BufferUPtr CDataWrapper::getBSONDataBuffer() const {
    return BufferUPtr(new Buffer(reinterpret_cast<const char*>(bson_get_data(ACCESS_BSON(bson))),
                      bson->len));
}

/*
 Return the pointer of the data and the size. th eownership of data remain
 of the CDataWrapper
 */
const char* CDataWrapper::getBSONRawData(int& size) const{
    if(bson.get()==NULL){
        return NULL;

    }
    size = bson->len;
    return reinterpret_cast<const char*>(bson_get_data(ACCESS_BSON(bson)));
}

/*
 Return the pointer of the data and the size. th eownership of data remain
 of the CDataWrapper
 */
const char* CDataWrapper::getBSONRawData() const{
    return reinterpret_cast<const char*>(bson_get_data(ACCESS_BSON(bson)));
}
const bson_t*CDataWrapper::getBSON() const {
    return reinterpret_cast<const bson_t*>((ACCESS_BSON(bson)));
    }

const int CDataWrapper::getBSONRawSize() const{
    return bson->len;
}

//return the json data
string CDataWrapper::getJSONString()  const{
    size_t str_size = 0;
    if(bson.get()==NULL){
        return std::string("{}");

    }
    char * str_c = bson_as_canonical_extended_json(ACCESS_BSON(bson),
                                                   &str_size);
    if(str_c==NULL){
        return std::string("{}");

    }                                             
    std::string result(str_c);
    bson_free(str_c);
    return result;
}

//return the json data
string CDataWrapper::getCompliantJSONString()  const{
    size_t str_size = 0;
    char * str_c = bson_as_relaxed_extended_json(ACCESS_BSON(bson),&str_size);
    if(str_c==NULL){
        str_c =bson_as_canonical_extended_json(ACCESS_BSON(bson), &str_size);
        if(str_c==NULL){
            return std::string("{}");
        }
        LERR_<<"## cannot convert to compliant JSON:'"<<str_c<<"'";
    }
    std::string result(str_c);
    bson_free(str_c);
    return result;
}

//reinitialize the object with bson data
void CDataWrapper::setSerializedData(const char* bson_data) {
    bson_iter_t it;
    size_t len = (size_t)BSON_UINT32_FROM_LE(*reinterpret_cast<const uint32_t *>(bson_data));
    if(!bson_iter_init_from_data(&it,
                                 reinterpret_cast<const uint8_t *>(bson_data),
                                 len)) return;
    while(bson_iter_next(&it)){
        bson_append_value(ACCESS_BSON(bson),
                          bson_iter_key(&it),
                          -1,
                          bson_iter_value(&it));
    }
}

//reinitialize the object with bson data
void CDataWrapper::setSerializedJsonData(const char* json_data) {
  bson_error_t err;
  size_t       len = (size_t)strlen(json_data);
  if (len) {
    bson = ALLOCATE_BSONT(bson_new_from_json((const uint8_t*)json_data,
                                             len,
                                             &err));
    if (bson == NULL) {
      std::stringstream ss;
      ss << "Error serializing json into bson: '"<<err.message<<"' json:'"<< json_data << "' ";
      throw CException(1, ss.str(), __PRETTY_FUNCTION__);
    }
  }
}

//append all elemento of an
void CDataWrapper::appendAllElement(CDataWrapper& src_cdw) {
    bson_iter_t it;
    bson_iter_init(&it, ACCESS_BSON(src_cdw.bson));
    while (bson_iter_next(&it)) {
        bson_append_value(ACCESS_BSON(bson),
                          bson_iter_key(&it),
                          -1,
                          bson_iter_value(&it));
    }
}

bool CDataWrapper::copyKeyTo(const std::string& key_to_copy,
                             CDataWrapper& destination) const {
    return copyKeyToNewKey(key_to_copy,
                           key_to_copy,
                           destination);
}

//!copy a key(with value) from this instance to another CDataWrapper witha new key
bool CDataWrapper::copyKeyToNewKey(const std::string& key_to_copy,
                                   const std::string& new_key,
                                   CDataWrapper& destination) const {
    bson_iter_t it;
    bson_iter_init(&it, ACCESS_BSON(bson));
    if(bson_iter_find_case(&it, key_to_copy.c_str()) == false) return false;
    bson_append_value(ACCESS_BSON(destination.bson),
                      new_key.c_str(),
                      (int)new_key.size(),
                      bson_iter_value(&it));
    return true;
}

void CDataWrapper::copyAllTo(CDataWrapper& destination) const {
    bson_iter_t it;
    bson_iter_init(&it, ACCESS_BSON(bson));
    while (bson_iter_next(&it)) {
        bson_append_value(ACCESS_BSON(destination.bson),
                          bson_iter_key(&it),
                          -1,
                          bson_iter_value(&it));
    }
}

//reset the datawrapper
void CDataWrapper::reset() {
    bson_reinit(ACCESS_BSON(bson));
    bson_tmp_array.reset();
}
bool CDataWrapper::removeKey(const std::string& key){
    CDataWrapper destination;
     bson_iter_t it;
     bool found=false;
    bson_iter_init(&it, ACCESS_BSON(bson));
    while (bson_iter_next(&it)) {
        if(bson_iter_key(&it)!=key){
            bson_append_value(ACCESS_BSON(destination.bson),
                          bson_iter_key(&it),
                          -1,
                          bson_iter_value(&it));
        } else {
            found=true;
        }
    }
    bson_reinit(ACCESS_BSON(bson));
    bson_iter_init(&it, ACCESS_BSON(destination.bson));
    while (bson_iter_next(&it)) {
            bson_append_value(ACCESS_BSON(bson),
                          bson_iter_key(&it),
                          -1,
                          bson_iter_value(&it));
        
    }
    return found;
}
bool CDataWrapper::replaceKey(const std::string& key,const CDataWrapper&d ){
    CDataWrapper destination;
     bson_iter_t it;
     bool found=false;
    bson_iter_init(&it, ACCESS_BSON(bson));
    while (bson_iter_next(&it)) {
        if(bson_iter_key(&it)!=key){
            bson_append_value(ACCESS_BSON(destination.bson),
                          bson_iter_key(&it),
                          -1,
                          bson_iter_value(&it));
        } else {
            bson_append_document(ACCESS_BSON(destination.bson),
                         key.c_str(),
                         (int)key.size(),
                         ACCESS_BSON(d.bson));
          
            found=true;
        }
    }
    reset();
    bson_iter_init(&it, ACCESS_BSON(destination.bson));
    while (bson_iter_next(&it)) {
            bson_append_value(ACCESS_BSON(bson),
                          bson_iter_key(&it),
                          -1,
                          bson_iter_value(&it));
        
    }
    return found;
}


string CDataWrapper::toHash() const{
    char ret[33];
    bson_md5_t m;
    uint8_t digest[16];
    bson_md5_init(&m);
    bson_md5_append(&m,
                    bson_get_data(ACCESS_BSON(bson)),
                    bson->len);
    bson_md5_finish(&m,
                    digest);
    memset(ret, 0, 33);
    for(int i = 0; i < 16; ++i) {
        sprintf(&ret[i*2], "%02x", (unsigned int)digest[i]);
    }
    return std::string(ret, 33);
}

CDataVariant CDataWrapper::getVariantValue(const std::string& key) const{
    //check if key is present
    if(!hasKey(key)) {
        LERR_<<__PRETTY_FUNCTION__<<" Key '"+key+"' not found ";
        return CDataVariant();
    }
    //create variant using the typed data
    switch (getValueType(key)) {
        case chaos::DataType::TYPE_BOOLEAN:
            return CDataVariant(getBoolValue(key));
        case  chaos::DataType::TYPE_INT32:
            return CDataVariant(getInt32Value(key));
        case  chaos::DataType::TYPE_INT64:
            return CDataVariant(getInt64Value(key));
        case  chaos::DataType::TYPE_UINT64:
            return CDataVariant(getUInt64Value(key));
        case  chaos::DataType::TYPE_DOUBLE:{
            double val=getDoubleValue(key);
            return CDataVariant(val);
        }
        case  chaos::DataType::TYPE_FLOAT:{
            float val=getDoubleValue(key);
            return CDataVariant(val);
        }
        case chaos::DataType::TYPE_STRING:
            return  CDataVariant(getStringValue(key));
        case chaos::DataType::TYPE_CLUSTER:
            if(isStringValue(key)){
                return CDataVariant(getValueType(key),(const void *)getStringValue(key).c_str(),(uint32_t)getStringValue(key).size()+1);
            } else {
                return CDataVariant(MOVE(getCSDataValue(key)));

            }
          
       
        case  chaos::DataType::TYPE_BYTEARRAY:
            return CDataVariant(getBinaryValueAsCDataBuffer(key));
        default:
               throw CException(-120, "Variant value type for key '"+key+"' unsupported", __PRETTY_FUNCTION__);

            break;
    }
    return CDataVariant();

}

//------------------------checking utility
bool CDataWrapper::isNullValue(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_NULL){
        return true;
    }
    return false;
}

bool CDataWrapper::isBoolValue(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_BOOL){
        return true;
    }
    return false;
}

bool CDataWrapper::isInt32Value(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_INT32){
        return true;
    }
    return false;
}

bool CDataWrapper::isInt64Value(const std::string& key) const{
    bson_iter_t element_found;
    bson_iter_init(&element_found, ACCESS_BSON(bson));
    if(bson_iter_find_case(&element_found, key.c_str())){
        if(BSON_ITER_HOLDS_INT64(&element_found)||BSON_ITER_HOLDS_TIMESTAMP(&element_found)){
            return true;
        }
    }
    return false;
}



bool CDataWrapper::isDoubleValue(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_DOUBLE){
        return true;
    }
    return false;
}

bool  CDataWrapper::isJsonValue(const std::string& json) const{
    bool ret=false;
    try{
        CDataWrapper tmp;
        tmp.setSerializedJsonData(json.c_str());
        ret=true;
    } catch(...){

    }
    return ret;
}

double CDataWrapper::getAsRealValue(const std::string& key) const{
    if(isDoubleValue(key)){
        return getDoubleValue(key);
    }
    if(isInt32Value(key)){
        return (double)getInt32Value(key);
    }
    if(isInt64Value(key)){
        return (double)getInt64Value(key);
    }
    if(isBoolValue(key)){
        return (double)getBoolValue(key);
    }
    return std::numeric_limits<double>::quiet_NaN();
}

void CDataWrapper::addJsonValue(const std::string& key, const string& val){
    CDataWrapper tmp;
    tmp.setSerializedJsonData(val.c_str());
    addCSDataValue(key,tmp);
}

/*void CDataWrapper::addJsonValue(const std::string& key, Json::Value& val){
    Json::FastWriter fastWriter;
    std::string output = fastWriter.write(val);
    addJsonValue(key,output);
}
*/
bool CDataWrapper::isStringValue(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_UTF8){
        return true;
    }
    return false;
}

bool CDataWrapper::isBinaryValue(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_BINARY){
        return true;
    }
    return false;
}
bool CDataWrapper::isCDataWrapperValue() const{
    bson_iter_t element_found;bson_iter_init(&element_found, ACCESS_BSON(bson));
    if(BSON_ITER_HOLDS_DOCUMENT(&element_found))return true;
   
    return false;
}

bool CDataWrapper::isCDataWrapperValue(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_DOCUMENT){
        return true;
    }
    return false;
}

bool CDataWrapper::isVectorValue(const std::string& key) const{
    FIND_AND_CHECK(key, BSON_ITER_HOLDS_ARRAY){
        return true;
    }
    return false;
}

chaos::DataType::DataType CDataWrapper::getValueType(const std::string& key) const{
    chaos::DataType::DataType  result =  chaos::DataType::TYPE_UNDEFINED;
    bson_iter_t it;
    bson_iter_init(&it, ACCESS_BSON(bson));
    if(bson_iter_find_case(&it, key.c_str()) == false) return result;
    switch(bson_iter_type(&it)) {
        case BSON_TYPE_ARRAY:
            result = chaos::DataType::TYPE_ACCESS_ARRAY;
            break;
        case BSON_TYPE_DOCUMENT:
            result = chaos::DataType::TYPE_CLUSTER;
            break;
        case BSON_TYPE_BINARY:
            result = chaos::DataType::TYPE_BYTEARRAY;
            switch(getBinarySubtype(key)){
                case chaos::DataType::SUB_TYPE_INT8:
                return chaos::DataType::TYPE_VECTOR_INT8;
                case chaos::DataType::SUB_TYPE_UINT8:
                return chaos::DataType::TYPE_VECTOR_UINT8;
                case chaos::DataType::SUB_TYPE_INT32:
                return chaos::DataType::TYPE_VECTOR_INT32;
                case chaos::DataType::SUB_TYPE_UINT32:
                return chaos::DataType::TYPE_VECTOR_UINT32;
                case chaos::DataType::SUB_TYPE_FLOAT:
                return chaos::DataType::TYPE_VECTOR_FLOAT;
                case chaos::DataType::SUB_TYPE_DOUBLE:
                return chaos::DataType::TYPE_VECTOR_DOUBLE;
                case chaos::DataType::SUB_TYPE_INT16:
                return chaos::DataType::TYPE_VECTOR_INT16;
                case chaos::DataType::SUB_TYPE_UINT16:
                return chaos::DataType::TYPE_VECTOR_UINT16;
                case chaos::DataType::SUB_TYPE_INT64:
                return chaos::DataType::TYPE_VECTOR_INT64;
                case chaos::DataType::SUB_TYPE_UINT64:
                return chaos::DataType::TYPE_VECTOR_UINT64;
                case chaos::DataType::SUB_TYPE_BOOLEAN:
                return chaos::DataType::TYPE_VECTOR_BOOL;     

            }
            return result;
        case BSON_TYPE_UTF8:
            result = chaos::DataType::TYPE_STRING;
            break;
        case BSON_TYPE_DOUBLE:
            result = chaos::DataType::TYPE_DOUBLE;
            break;
        case BSON_TYPE_INT32:
            result = chaos::DataType::TYPE_INT32;
            break;
        case BSON_TYPE_INT64:
            result = chaos::DataType::TYPE_INT64;
            break;
        case BSON_TYPE_TIMESTAMP:
            result = chaos::DataType::TYPE_UINT64;
            break;
        case BSON_TYPE_BOOL:
            result = chaos::DataType::TYPE_BOOLEAN;
            break;
        case BSON_TYPE_NULL:
            result = chaos::DataType::TYPE_UNDEFINED;
            break;
        default:
            break;
            
    }
    return result;
}

bool CDataWrapper::isEmpty() const {
    return (bson_count_keys(ACCESS_BSON(bson)) == 0);
}

int CDataWrapper::setAsString(const std::string& key,const std::string& sval){
     bson_iter_t it;
    bson_iter_init(&it, static_cast<bson_t*>(bson.get()));
    if(bson_iter_find_case(&it, key.c_str()) == false)
        return -1;
    switch(ITER_TYPE(&it)){
        case BSON_TYPE_INT64:{
            CDataVariant val(sval);

            int64_t tmp=val.asInt64();
            setBson(&it,tmp);
        }
        break;
       /* case BSON_TYPE_TIMESTAMP:{
            CDataVariant val(sval);

            uint64_t tmp=val.asUInt64();
            setBson(&it,tmp);
        }
        break;*/
         case BSON_TYPE_DOUBLE:{
            CDataVariant val(sval);

            double tmp=val.asDouble();
            setBson(&it,tmp);
        }
        break;
        case BSON_TYPE_INT32:{
            CDataVariant val(sval);

            int32_t tmp=val.asInt32();
            setBson(&it,tmp);
        }
        break;
         case BSON_TYPE_BOOL:{
            CDataVariant val(sval);

            bool tmp=val.asBool();
            setBson(&it,tmp);
        }
        break;
        case BSON_TYPE_UTF8:
            setBson(&it,sval);
            break;
        case BSON_TYPE_BINARY:{

            chaos::common::data::CDBufferUniquePtr ptr=Base64Util::decode(sval);

            setBson(&it,ptr->getBuffer(),ptr->getBufferSize());
            break;
        }
        default:
        return -2;
        

    }
    return 0;
}


int CDataWrapper::setBson( const bson_iter_t *v ,const int64_t& val){
    if(ITER_TYPE(v)==BSON_TYPE_INT64){
        memcpy((void*)(v->raw + v->d1), (void*)&val,sizeof(int64_t));
        return sizeof(int64_t);
    }
    return -1;
}

int CDataWrapper::setBson(const bson_iter_t *v ,const int32_t& val){
    if(ITER_TYPE(v)==BSON_TYPE_INT32){
        memcpy((void*)(v->raw + v->d1),(void*)&val ,sizeof(int32_t));
        return sizeof(int32_t);
    }
    return -1;
}

int CDataWrapper::setBson(const bson_iter_t *v ,const double& val){
    if(ITER_TYPE(v)==BSON_TYPE_DOUBLE){
        memcpy((void*)(v->raw + v->d1), (void*)&val,sizeof(double));
        return sizeof(double);
    }
    return -1;
}

int CDataWrapper::setBson(const bson_iter_t *v ,const bool& val){
    if(ITER_TYPE(v)==BSON_TYPE_BOOL){
        memcpy((void*)(v->raw + v->d1), (void*)&val,sizeof(bool));
        return sizeof(bool);
    }
    return -1;
}

int CDataWrapper::setBson(const bson_iter_t *v ,const std::string& val){
    if(ITER_TYPE(v)== BSON_TYPE_UTF8){
        char key[256];
        strncpy(key,bson_iter_key_unsafe(v),sizeof(key));
        removeKey(key);
        addStringValue(key,val);
    
        return (val.size()+1);

    }
    return -1;
}

int CDataWrapper::setBson(const bson_iter_t *v ,const void* val){
    if(ITER_TYPE(v)== BSON_TYPE_BINARY){
        const bson_value_t *vv = bson_iter_value((bson_iter_t *)v);
        memcpy((void*)(v->raw + v->d3), (void*)val,vv->value.v_binary.data_len);
        return vv->value.v_binary.data_len;
    } else if(ITER_TYPE(v)== BSON_TYPE_UTF8){
        return setBson(v,std::string((const char*)val));
    }
    return -1;
}
int CDataWrapper::setValue(const std::string& key,const void* val,size_t size){
     bson_iter_t it;
     bson_iter_init(&it, static_cast<bson_t*>(bson.get()));
    if(bson_iter_find_case(&it, key.c_str()) == false)
        return -1;
    return setBson(&it,val,size);
}

             
int CDataWrapper::setBson(const bson_iter_t *v ,const void* val,size_t size){
    if(ITER_TYPE(v)== BSON_TYPE_BINARY){
         bson_value_t *vv = (bson_value_t *)bson_iter_value((bson_iter_t *)v);
        // without check is more useful, the programmer must be aware of the preallocated data size
        if(size>=vv->value.v_binary.data_len){
            char key[256];
            strncpy(key,bson_iter_key_unsafe(v),sizeof(key));
            removeKey(key);
            addBinaryValue(key,(const char*)val,size);
            return size;
        }  else {
            memcpy((void*)(v->raw + v->d3), (void*)val,size);
            vv->value.v_binary.data_len=size;
            return vv->value.v_binary.data_len;
        }
    }
    return -1;
}
int CDataWrapper::setBson(const bson_iter_t *v ,const CDataWrapper* val){
    if(ITER_TYPE(v)== BSON_TYPE_DOCUMENT){
        const bson_value_t *vv = bson_iter_value((bson_iter_t *)v);
        
        memcpy((void*)(v->raw + v->d1), (void*)val->getBSONRawData(),vv->value.v_doc.data_len);
        return vv->value.v_doc.data_len;
    }
    return -1;
}
#ifdef EPICS
    void CDataWrapper::setSerializedData(epics::pvData::StructureConstPtr ptr){
        	std::string id= ptr->getID();
            addStringValue("type",id);
             size_t numberFields = ptr->getNumberFields();
            for(size_t i=0; i<numberFields; i++) {
             epics::pvData::FieldConstPtr pfield = ptr->getField(i);
             CDataWrapper cs;
            cs.addStringValue(ptr->getFieldName(i),pfield->getID());

            switch(pfield->getType()) {
            case epics::pvData::scalar:
            case epics::pvData::scalarArray:
                break;
            case epics::pvData::structure:
            {
                epics::pvData::Field const *xxx = pfield.get();
                epics::pvData::Structure const *pstruct = static_cast<epics::pvData::Structure const*>(xxx);
                cs.setSerializedData(epics::pvData::StructureConstPtr(pstruct));
                break;
            }
           /* case epics::pvData::structureArray:
            {
                format::indent_scope s(o);
                Field const *xxx = pfield.get();
                StructureArray const *pstructureArray = static_cast<StructureArray const*>(xxx);
                o << *pstructureArray->getStructure();
                break;
            }
            case epics::pvData::union_:
            {
                Field const *xxx = pfield.get();
                Union const *punion = static_cast<Union const*>(xxx);
                format::indent_scope s(o);
                punion->dumpFields(o);
                break;
            }
            case epics::pvData::unionArray:
            {
                format::indent_scope s(o);
                Field const *xxx = pfield.get();
                UnionArray const *punionArray = static_cast<UnionArray const*>(xxx);
                o << *punionArray->getUnion();
                break;
            }*/
            default:
                break;
        }
    }
          
    }

#endif

#pragma mark CMultiTypeDataArrayWrapper
CMultiTypeDataArrayWrapper::CMultiTypeDataArrayWrapper(const ChaosBsonShrdPtr& _document_shrd_ptr,
                                                       const std::string& key):
document_shrd_ptr(_document_shrd_ptr),
array_doc(new bson_t()) {
    bson_iter_t element_found;
    bson_iter_init(&element_found, ACCESS_BSON(_document_shrd_ptr));
    if(bson_iter_find_case(&element_found, key.c_str())&&
       BSON_ITER_HOLDS_ARRAY(&element_found)) {
        uint32_t array_len;
        const uint8_t *array;
        bson_iter_array(&element_found,
                        &array_len,
                        &array);
        if (bson_init_static(array_doc, array, array_len)) {
            bson_iter_t iter;
            if(bson_iter_init(&iter, array_doc)) {
                while(bson_iter_next(&iter)) {
                    //ChaosBsonValuesShrdPtr copy = ChaosBsonValuesShrdPtr(new bson_value_t(), &bsonValueDestroy);
                    //bson_value_t copy;
                    bson_value_t*copy = new bson_value_t();
                    bson_value_copy(bson_iter_value(&iter), copy);
                    values.push_back(copy);
                }
            }
        }
    }
}
std::map<std::string,std::string> CMultiTypeDataArrayWrapper::toKVmap(const std::string kname,const std::string kvalue) const {
std::map<std::string,std::string> ret;
    for(int cnt=0;cnt<size();cnt++){
        if(isCDataWrapperElementAtIndex(cnt)){
           CDWUniquePtr ele= getCDataWrapperElementAtIndex(cnt);
           if(ele->hasKey(kname)&& ele->hasKey(kvalue)){
               std::string k=ele->getStringValue(kname);
               if(k.size()){
                 ret[k]=ele->getStringValue(kvalue);
               }
           }
        }
    }
    return ret;
}

CMultiTypeDataArrayWrapper::~CMultiTypeDataArrayWrapper() {
    for(VectorBsonValuesIterator it = values.begin(),
        end = values.end();
        it != end;
        it++) {
        bson_value_destroy(*it);
        delete(*it);
    }
    values.clear();
    delete(array_doc);
}

std::string CMultiTypeDataArrayWrapper::getJSONString() {
    size_t str_size;
    char * str_c = bson_as_canonical_extended_json(static_cast<const bson_t*>(array_doc),&str_size);
    return std::string(str_c,str_size);
}

std::string CMultiTypeDataArrayWrapper::getCanonicalJSONString() {
    size_t str_size;
    char * str_c = bson_as_relaxed_extended_json(static_cast<const bson_t*>(array_doc),&str_size);
    return std::string(str_c,str_size);
}

string CMultiTypeDataArrayWrapper::getStringElementAtIndex(const int pos) const{
    //   CHAOS_ASSERT(values[pos]->value_type == BSON_TYPE_UTF8);
    if(values[pos]->value_type != BSON_TYPE_UTF8){
        std::stringstream ss;
        ss<<"type at index ["<<pos<<"] is not String, typeid:"<<values[pos]->value_type;
        throw CException(1, ss.str(), __PRETTY_FUNCTION__);
    }
    return std::string(values[pos]->value.v_utf8.str, values[pos]->value.v_utf8.len);
}

bson_value_t * CMultiTypeDataArrayWrapper::getBSONElementAtIndex(const int pos) const{
    if(pos<values.size()) return values[pos];
    return NULL;
}

double CMultiTypeDataArrayWrapper::getDoubleElementAtIndex(const int pos) const{
    if(values[pos]->value_type != BSON_TYPE_DOUBLE){
        std::stringstream ss;
        ss<<"type at index ["<<pos<<"] is not double, typeid:"<<values[pos]->value_type;
        throw CException(1, ss.str(), __PRETTY_FUNCTION__);
    }
    return values[pos]->value.v_double;
}

int32_t CMultiTypeDataArrayWrapper::getInt32ElementAtIndex(const int pos) const{
    return values[pos]->value.v_int32;
}

bool CMultiTypeDataArrayWrapper::getBoolElementAtIndex(const int pos) const{
    return values[pos]->value.v_bool;
    
    
}
int CMultiTypeDataArrayWrapper::removeElementAtIndex(const int pos) {
    if(pos<values.size()){
        VectorBsonValues::iterator i=values.begin()+pos;
        bson_value_destroy(*i);
        values.erase(i);
    }
    return values.size();
}

int64_t CMultiTypeDataArrayWrapper::getInt64ElementAtIndex(const int pos) const{
    //CHAOS_ASSERT(values[pos]->value_type == BSON_TYPE_INT64);
    if(values[pos]->value_type != BSON_TYPE_INT64){
        std::stringstream ss;
        ss<<"type at index ["<<pos<<"] is not int64, typeid:"<<values[pos]->value_type;
        throw CException(1, ss.str(), __PRETTY_FUNCTION__);
    }
    return values[pos]->value.v_int64;
}

uint64_t CMultiTypeDataArrayWrapper::getUInt64ElementAtIndex(const int pos) const{
    //CHAOS_ASSERT(values[pos]->value_type == BSON_TYPE_INT64);
    if(values[pos]->value_type != BSON_TYPE_TIMESTAMP){
        std::stringstream ss;
        ss<<"type at index ["<<pos<<"] is not uint64, typeid:"<<values[pos]->value_type;
        throw CException(1, ss.str(), __PRETTY_FUNCTION__);
    }
     uint64_t ret=((uint64_t)values[pos]->value.v_timestamp.timestamp<<32) | values[pos]->value.v_timestamp.increment;

    return ret;
}

bool CMultiTypeDataArrayWrapper::isStringElementAtIndex(const int pos) const{
    return values[pos]->value_type == BSON_TYPE_UTF8;
}

bool CMultiTypeDataArrayWrapper::isDoubleElementAtIndex(const int pos) const{
    return values[pos]->value_type == BSON_TYPE_DOUBLE;
}

bool CMultiTypeDataArrayWrapper::isInt32ElementAtIndex(const int pos) const{
    return values[pos]->value_type == BSON_TYPE_INT32;
}
bool CMultiTypeDataArrayWrapper::isBoolElementAtIndex(const int pos) const{
    return values[pos]->value_type == BSON_TYPE_BOOL;
}
bool CMultiTypeDataArrayWrapper::isInt64ElementAtIndex(const int pos) const{
    return ((values[pos]->value_type == BSON_TYPE_INT64)||(values[pos]->value_type == BSON_TYPE_TIMESTAMP));
}

bool CMultiTypeDataArrayWrapper::isCDataWrapperElementAtIndex(const int pos) const{
    return values[pos]->value_type == BSON_TYPE_DOCUMENT;
}

const char * CMultiTypeDataArrayWrapper::getRawValueAtIndex(const int pos,uint32_t& size) const{
    switch(values[pos]->value_type ) {
        case BSON_TYPE_INT64:
            size=sizeof(int64_t);
            return reinterpret_cast<const char*>(&values[pos]->value.v_int64);
        case BSON_TYPE_TIMESTAMP:
            size=sizeof(uint64_t);
            return reinterpret_cast<const char*>(&values[pos]->value.v_timestamp);
        case BSON_TYPE_INT32:
            size=sizeof(int32_t);
            return reinterpret_cast<const char*>(&values[pos]->value.v_int32);
        case BSON_TYPE_BOOL:
            size=sizeof(bool);
            return reinterpret_cast<const char*>(&values[pos]->value.v_bool);
        case BSON_TYPE_DOUBLE:
            size=sizeof(double);
            return reinterpret_cast<const char*>(&values[pos]->value.v_double);
        case BSON_TYPE_UTF8:
            size = values[pos]->value.v_utf8.len;
            return static_cast<const char*>(values[pos]->value.v_utf8.str);
        case BSON_TYPE_BINARY:
            size = values[pos]->value.v_binary.data_len;
            return reinterpret_cast<const char*>(values[pos]->value.v_binary.data);
        default:
            break;
    }
    return NULL;
}

ChaosUniquePtr<CDataWrapper> CMultiTypeDataArrayWrapper::getCDataWrapperElementAtIndex(const int pos) const{
    // CHAOS_ASSERT(values[pos]->value_type == BSON_TYPE_DOCUMENT);
    if(values[pos]->value_type != BSON_TYPE_DOCUMENT){
        std::stringstream ss;
        ss<<"type at index ["<<pos<<"] is not CDataWrapper, typeid:"<<values[pos]->value_type;
        throw CException(1, ss.str(), __PRETTY_FUNCTION__);
    }
    return ChaosUniquePtr<CDataWrapper>(new CDataWrapper((const char *)values[pos]->value.v_doc.data, values[pos]->value.v_doc.data_len));
}

size_t CMultiTypeDataArrayWrapper::size() const{
    return values.size();
}
 