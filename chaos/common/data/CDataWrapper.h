/*
 *	CDataWrapper.h
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
#ifndef CDataWrapper_H
#define CDataWrapper_H

#include <chaos/common/chaos_types.h>
#include <chaos/common/chaos_constants.h>
#include <chaos/common/bson/bson.h>
#include <chaos/common/exception/CException.h>
#include "CDataBuffer.h"
#include "CDataVariant.h"
#include <utility>

#ifdef _WIN32
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__  __FUNCSIG__
#endif
#endif

#ifdef EPICS
namespace epics{
    namespace pvData{
        class Structure;
        class PVUnion;
        class PVStructure;
        class PVField;
#if __cplusplus >= 201103L
        typedef std::shared_ptr<const Structure> StructureConstPtr;
        typedef std::shared_ptr<const PVUnion> PVUnionConstPtr;
        typedef std::shared_ptr<const PVStructure> PVStructureConstPtr;
        typedef std::shared_ptr<const PVField> PVFieldConstPtr;



#else
        typedef std::tr1::shared_ptr<const Structure> StructureConstPtr;

#endif
    }
}
#endif

#if defined(__GNUC__) && (__GNUC__ >= 6) && !defined(__clang__)
// See libmongoc.hh for details on this diagnostic suppression
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#endif

namespace chaos {
    namespace common {
        namespace data {
            using namespace std;
            class CDataWrapper;
            /*!
             Class to read the and arry of multivalue
             */
            class CMultiTypeDataArrayWrapper;
            typedef ChaosUniquePtr<CMultiTypeDataArrayWrapper> CMultiTypeDataArrayWrapperUPtr;
            typedef ChaosSharedPtr<CMultiTypeDataArrayWrapper> CMultiTypeDataArrayWrapperSPtr;

            /*!
             Class for contain the serialization buffer
             the class deallocation will dealloc all the
             serialization buffer
             */
            class SerializationBuffer {
                size_t bSize;
                char *buffer;
            public:
                bool disposeOnDelete;
                SerializationBuffer(const char *iBuff, size_t iSize) {
                    bSize = iSize;
                    buffer = 0L;
                    disposeOnDelete = true;
                    if(iBuff && iSize){
                        buffer = (char*)new char[iSize];
                        std::memcpy(buffer, iBuff, iSize);
                    }
                }
                ~SerializationBuffer(){
                    if(disposeOnDelete && buffer) delete [](buffer);
                }
                size_t getBufferLen(){return bSize;};
                const char *getBufferPtr(){return buffer;};
            };
            typedef ChaosUniquePtr<SerializationBuffer> SerializationBufferUPtr;
            typedef ChaosSharedPtr<struct _bson_t> ChaosBsonShrdPtr;
            typedef ChaosSharedPtr<struct _bson_value_t> ChaosBsonValuesShrdPtr;

            CHAOS_DEFINE_VECTOR_FOR_TYPE(bson_value_t*, VectorBsonValues);



 /*!
             Class to read the and arry of multivalue
             */
            class CMultiTypeDataArrayWrapper {
                friend class CDataWrapper;
                const ChaosBsonShrdPtr document_shrd_ptr;
                bson_t *array_doc;
                VectorBsonValues values;
                CMultiTypeDataArrayWrapper(const ChaosBsonShrdPtr& _document_shrd_ptr,
                                           const std::string& key);
            public:
                ~CMultiTypeDataArrayWrapper();
                string getStringElementAtIndex(const int) const;
                double getDoubleElementAtIndex(const int) const;
                int32_t getInt32ElementAtIndex(const int) const;
                int64_t getInt64ElementAtIndex(const int) const;
                uint64_t getUInt64ElementAtIndex(const int) const;
                bson_value_t * getBSONElementAtIndex(const int pos) const;
                bool getBoolElementAtIndex(const int) const;
                /**
                 * @brief convert an array of cdwappers with k,v into a map 
                 * 
                 * @return std::map<std::string,std::string> 
                 */
                std::map<std::string,std::string> toKVmap(const std::string kname="name",const std::string kvalue="value") const;
               
                ChaosUniquePtr<CDataWrapper> getCDataWrapperElementAtIndex(const int) const;
                std::string getJSONString();
                std::string getCanonicalJSONString();
                bool isStringElementAtIndex(const int) const;
                bool isDoubleElementAtIndex(const int) const;
                bool isInt32ElementAtIndex(const int) const;
                bool isInt64ElementAtIndex(const int) const;
                bool isBoolElementAtIndex(const int) const;
                int removeElementAtIndex(const int);
                bool isCDataWrapperElementAtIndex(const int) const;
                template<class T>
                T getElementAtIndex(const int pos) const{
                    if(values[pos]->value_type == BSON_TYPE_DOUBLE){
                        return static_cast<T>(values[pos]->value.v_double);
                    }
                    if(values[pos]->value_type == BSON_TYPE_INT32){
                        return static_cast<T>(values[pos]->value.v_int32);
                    }
                    if(values[pos]->value_type == BSON_TYPE_INT64){
                        return static_cast<T>(values[pos]->value.v_int64);
                    }
                    if(values[pos]->value_type == BSON_TYPE_TIMESTAMP){
                        uint64_t ret=((uint64_t)values[pos]->value.v_timestamp.timestamp<<32) | values[pos]->value.v_timestamp.increment;

                        
                        return static_cast<T>(ret);
                    }
                    if(values[pos]->value_type == BSON_TYPE_BOOL){
                        return static_cast<T>(values[pos]->value.v_bool);
                    }
                    std::stringstream ss;
                    ss<<"type at index ["<<pos<<"] cannot convert, typeid:"<<values[pos]->value_type;
                    throw CException(1, ss.str(), __PRETTY_FUNCTION__);
                    return 0;
                }

                operator std::vector<std::string>(){
                    std::vector<std::string> ret;
                    for(int cnt=0;cnt<size();cnt++){
                        ret.push_back(getStringElementAtIndex(cnt));
                    }
                    return ret;
                }
                 operator std::set<std::string>(){
                    std::set<std::string> ret;
                    for(int cnt=0;cnt<size();cnt++){
                        ret.insert(getStringElementAtIndex(cnt));
                    }
                    return ret;
                }
                const char * getRawValueAtIndex(const int key,uint32_t& size) const;
                size_t size() const;
            };
            /*!
             Class that wrap the serializaiton system for data storage
             */
            class CDataWrapper {
                ChaosBsonShrdPtr bson;
                int array_index;
                ChaosBsonShrdPtr bson_tmp_array;
                explicit CDataWrapper(const std::string& json_document);
               // int setBson(const bson_iter_t * ,const uint64_t& val);
                int setBson(const bson_iter_t * ,const int64_t& val);
                int setBson(const bson_iter_t *v ,const int32_t& val);
                int setBson(const bson_iter_t * ,const double& val);
                int setBson(const bson_iter_t *,const bool& val);
                int setBson(const bson_iter_t * ,const std::string& val);
                int setBson(const bson_iter_t * ,const void* val);
                int setBson(const bson_iter_t *v ,const void* val,size_t size);

                int setBson(const bson_iter_t *v ,const CDataWrapper* val);

            public:
            static bool isJSON(const::std::string&str);
                CDataWrapper();
                CDataWrapper(const bson_t *copy_bson);

                explicit CDataWrapper(const char* mem_ser,
                                      uint32_t mem_size);
                explicit CDataWrapper(const char* mem_ser);
                ~CDataWrapper();
                
                const bson_t*getBSON() const;

                static ChaosUniquePtr<CDataWrapper> instanceFromJson(const std::string& json_serialization);
                ChaosUniquePtr<CDataWrapper>clone() const;
                //add a csdata value
                void addCSDataValue(const std::string&, const CDataWrapper&);
                //get a csdata value
                ChaosUniquePtr<chaos::common::data::CDataWrapper> getCSDataValue(const std::string&) const;
                void getCSDataValue(const std::string&,chaos::common::data::CDataWrapper&) const;


                //get a projection of a vector of keys
                ChaosUniquePtr<chaos::common::data::CDataWrapper> getCSProjection(const std::vector<std::string>&) const;

                #ifdef EPICS
                    void setSerializedData(epics::pvData::PVStructureConstPtr ptr);
                    void setSerializedData(epics::pvData::PVUnionConstPtr ptr);
                    void decodePVField(epics::pvData::PVFieldConstPtr);

                #endif
                //add a string value
                //void addStringValue(const char *, const char *); 
                /**
                 * @brief Add a string value to the Wrapper
                 * 
                 * @param key the associated key
                 * @param val the value
                 * @param max_size the max allocated size (if 0 the allocated size will be same as the string)
                 */
                void addStringValue(const std::string& key, const string& val, const int max_size=0);

                
                //add a json value
                void addJsonValue(const std::string&, const string&);
                //add a json value
            //    void addJsonValue(const std::string&, Json::Value&);
                //add a strin gto array
                void appendStringToArray(const string &value);
                void appendInt32ToArray(int32_t value);
                void appendInt64ToArray(int64_t value);
                void appendUInt64ToArray(uint64_t value);

                void appendDoubleToArray(double value);
                void appendBooleanToArray(bool value);
                void appendCDataWrapperToArray(const CDataWrapper& value);
                //finalize the array into a key for the current dataobject
                void finalizeArrayForKey(const std::string&);
                //get a string value
                string  getStringValue(const std::string&) const;
                const char *  getCStringValue(const std::string& key) const;
                //return a vectorvalue for a key
                CMultiTypeDataArrayWrapperSPtr getVectorValue(const std::string&) const;
                void addNullValue(const std::string&);
                
                void append(const std::string& key,int32_t val);
                void append(const std::string& key,int64_t val);

                void append(const std::string& key,double val);
                void append(const std::string& key,bool val);
                void append(const std::string& key,const char* val);
                void append(const std::string&key,CMultiTypeDataArrayWrapperSPtr&) ;

                void append(const std::string& key,const std::string& val);
                void append(const std::string& key,const CDataWrapper& val);
              
                void append(const std::string& key,const std::vector<int32_t>& val);                
                void append(const std::string& key,const std::vector<int64_t>& val);
                void append(const std::string& key,const std::vector<double>& val);
                void append(const std::string& key,const std::vector<bool>& val);
                
                void append(const std::string&key,DataType::DataType typ,const char*buf,int len);
                void append(const std::string& key,const std::vector<std::string>& val);
                void append(const std::string& key,const std::vector<CDataWrapper>& val);
                
                template<typename T>
                int getVectorValue(const std::string& key,std::vector<T>&vv){
                    
                    CMultiTypeDataArrayWrapperSPtr v=getVectorValue(key);
                    for(int cnt=0;cnt<v->size();cnt++){
                        vv.push_back(v->getElementAtIndex<T>(cnt));
                            
                    }
                    return vv.size();
                    
                }
                /**
                * Array are realized as a binary sequence of bytes
                * 
               */
                template<typename T>
                    void appendArray(const std::string& key,const std::vector<T>& val){
                            appendArray(key,(T*)&val[0],val.size());

                    }
                void appendArray(const std::string& key,bool* arr,int count);
                void appendArray(const std::string& key,char* arr,int count);
                void appendArray(const std::string& key,int32_t* arr,int count);
                void appendArray(const std::string& key,uint32_t* arr,int count);

                void appendArray(const std::string& key,double* arr,int count);
                void appendArray(const std::string& key,float* arr,int count);

                void appendArray(const std::string& key,int16_t* arr,int count);
                void appendArray(const std::string& key,uint16_t* arr,int count);

                void appendArray(const std::string& key,int8_t* arr,int count);
                void appendArray(const std::string& key,uint8_t* arr,int count);
                
                void appendArray(const std::string& key,int64_t* arr,int count);
                void appendArray(const std::string& key,uint64_t* arr,int count);
                template<typename T>
                int getArrayValue(const std::string& key,std::vector<T>&v){
                    uint32_t bufLen;
                    T* ptr=(T*)getBinaryValue(key,bufLen);
                    if(ptr==NULL || bufLen==0){
                        return 0;
                    }
                    for(int cnt=0;cnt<bufLen/sizeof(T);cnt++){
                        v.push_back(ptr[cnt]);
                    }
                    return v.size();
                    
                }

                //add a integer value
                void addInt32Value(const std::string&, int32_t);
                //add a integer value
                void addUInt32Value(const std::string&, uint32_t);
                //add a integer value
                void addInt64Value(const std::string&, int64_t);
                //add a integer value
                void addUInt64Value(const std::string&, uint64_t);
                //add a double value
                void addDoubleValue(const std::string&key, double dValue);
                //add a bool value
                void addBoolValue(const std::string&, bool);
                //set a binary data value
                void addBinaryValue(const std::string&, const char *, int);
                //!add a value from variant
                void addVariantValue(const std::string& key,
                                     const CDataVariant& variant_value);
                //get a integer value
                int32_t getInt32Value(const std::string& key) const;
                //get a integer value
                int64_t getInt64Value(const std::string& key) const;
                //get a unsigned integer64 value
                uint64_t getUInt64Value(const std::string& key) const;
                //get a integer value
                uint32_t getUInt32Value(const std::string& key) const;
                
                //add a integer value
                double getDoubleValue(const std::string& key) const;
                //get a bool value
                bool getBoolValue(const std::string&) const;
                //get a json value
                std::string getJsonValue(const std::string&) const;
                // return key as a double value or nan if cannot convert
                double getAsRealValue(const std::string& key) const;
                
#define THROW_TYPE_EXC(type)\
std::stringstream ss;\
ss<<"cannot get or cast to '" << #type<<"'";\
throw chaos::CException(-2, ss.str(), __PRETTY_FUNCTION__);
                int setValue(const std::string& key,const void* val,size_t size);

                template<typename T>
                int setValue(const std::string& key,const T& val){
                    bson_iter_t it;
                    bson_iter_init(&it, static_cast<bson_t*>(bson.get()));
                    if(bson_iter_find_case(&it, key.c_str()) == false)
                        return -1;
                    //const bson_value_t *v = bson_iter_value(&it);
                    return setBson(&it,val);
                }
                /**
                 * @brief Set the As String in place use a string as container of data
                 * 
                 * @param key 
                 * @param val string value
                 * @return 0 if success
                 */
                int setAsString(const std::string& key,const std::string& val);

                template<typename T>
                int setValue(const std::string& key,std::vector<T>& val){
                    bson_iter_t it;
                    bson_iter_init(&it, static_cast<bson_t*>(bson.get()));
                    if(bson_iter_find_case(&it, key.c_str()) == false)
                        return -1;
                    const bson_value_t *v = bson_iter_value(&it);
                    if(v->value_type ==BSON_TYPE_ARRAY){
                        uint32_t array_len = 0;
                        const uint8_t *array = NULL;
                        bson_t array_doc;
                        bson_iter_array(&it, &array_len, &array);
                        if (bson_init_static(&array_doc, array, array_len)) {
                            bson_iter_t iter;
                            int cnt=0;
                            int size=val.size();
                            int ret=0;
                            if(bson_iter_init(&iter, &array_doc)) {
                                while(bson_iter_next(&iter)&& (cnt<size)) {
                                    int s;
                                    //const bson_value_t *v = bson_iter_value(&it);
                                    if((s=setBson(&iter,val[cnt]))>0){
                                        ret+=s;
                                    } else {
                                        return -1;
                                    }
                                    cnt++;
                                }
                                return ret;
                            }
                        }
                    }
                    return -1;
                }
                template<typename T>
                T getValue(const std::string& key) const{

                    T v;
                    if(hasKey(key) == false) {throw chaos::CException(-1, "Key not present", __PRETTY_FUNCTION__);}
                    v=(T)getVariantValue(key);
                   
                    return v;
                }
               /**
                * @brief Get the Binary Value corresponding to a key
                * 
                * @param key key of the binary
                * @param size returned size
                * @return NULL if errot
                */
                const char* getBinaryValue(const std::string&key , uint32_t& size) const;
                /**
                 * @brief Get the Binary corresponding to a key, but search also for base64 strings
                 * 
                 * @param key key of the binary
                 * @return CDBufferUniquePtr object
                 */
                
                CDBufferUniquePtr getBinaryValueAsCDataBuffer(const std::string &key) const;

                chaos::DataType::BinarySubtype getBinarySubtype(const std::string& key) const;
                void addBinaryValue(const std::string& key,
                                    chaos::DataType::BinarySubtype sub_type,
                                    const char *buff,
                                    int bufLen);

                
 
               

                template<typename T>
                int getArray(const std::string& key,T* arr,int count=-1){
                    int ret=0;
                    uint32_t size;
                    if(arr==NULL){
                        return -1;
                    }
                    const char* ptr=getBinaryValue(key , size);
                    int rsize=count*sizeof(T);
                   
                    if(ptr==NULL){
                        return -2;
                    }
                    if(rsize>=0){   
                        memcpy((void*)arr,ptr,rsize);
                        return count;
                    }
                    memcpy((void*)arr,ptr,size);
                    return size/sizeof(T);

                }
                template<typename T>
                std::vector<T> getArray(const std::string& key){
                     std::vector<T> res;
                     uint32_t size;
                    const char* ptr=getBinaryValue(key , size);
                    int count=size/sizeof(T);
                    for(int cnt=0;cnt<count;cnt++){
                        T tmp=((T*)ptr)[cnt];
                        res.push_back((tmp));
                    }
                    return res;
                   
                }

               

                //return the bson data
                SerializationBufferUPtr getBSONData() const;
                BufferUPtr getBSONDataBuffer() const;
                const char* getBSONRawData(int& size) const;
                const char* getBSONRawData() const;
                const int getBSONRawSize() const;
                chaos::common::data::ChaosBsonShrdPtr getBSONShrPtr() const { return bson;}

                //return the json data
                //SerializationBuffer* getJSONData();
                //return the json representation for this data wrapper
                string getJSONString() const;
                //return a compliatn json serialization
                string getCompliantJSONString() const;
                //reinitialize the object with bson data
                void setSerializedData(const char* bsonData);
                //reinitialize the object with bson data
                void setSerializedJsonData(const char* jsonData);
                //check if the key is present in data wrapper
                bool hasKey(const std::string& key) const;
                //
                bool removeKey(const std::string& key);
                bool replaceKey(const std::string& key,const CDataWrapper&d );

                bool isVector(const std::string& key) const;
                //return all key contained into the object
                void getAllKey(ChaosStringVector& contained_key) const;
                //return all key contained into the object
                void getAllKey(ChaosStringSet& contained_key) const;
                int countKeys() const;
                ChaosStringVector getAllKey() const;

                //return all key contained into the object
                uint32_t getValueSize(const std::string& key) const;
                //! get raw value ptr address
                const char * getRawValuePtr(const std::string& key) const;
                //reset the datawrapper
                void reset();
                //append all element of an data wrapper
                void appendAllElement(CDataWrapper&);
                //!copy a key(with value) from this instance to another CDataWrapper one
                bool copyKeyTo(const std::string& key_to_copy,
                               CDataWrapper& destination) const;
                //!copy a key(with value) from this instance to another CDataWrapper witha new key
                bool copyKeyToNewKey(const std::string& key_to_copy,
                                     const std::string& new_key,
                                     CDataWrapper& destination) const;
                //!copy all key(with value) from this instance to another CDataWrapper one
                void copyAllTo(CDataWrapper& destination) const;
                //! Return the Hashing represetnation of the CDataWrapper
                string toHash() const;
                CDataVariant getVariantValue(const std::string& key) const;
                //---checking funciton
                bool isNullValue(const std::string& key) const;
                bool isBoolValue(const std::string& key) const;
                bool isInt32Value(const std::string& key) const;
                bool isInt64Value(const std::string& key) const;
                bool isDoubleValue(const std::string& key) const;
                bool isStringValue(const std::string& key) const;
                bool isBinaryValue(const std::string& key) const;
                bool isCDataWrapperValue(const std::string& key) const;
                bool isCDataWrapperValue() const;

                bool isVectorValue(const std::string& key) const;
                bool isJsonValue(const std::string& key) const;
                chaos::DataType::DataType getValueType(const std::string& key) const;
                bool isEmpty() const;
                bool operator==(const CDataWrapper&d) const;
                bool operator!=(const CDataWrapper&d) const {return !(*this==d);};

            };
           

#define CDW_GET_SRT_WITH_DEFAULT(c, k, d) ((c)->hasKey(k)?(c)->getStringValue(k):d)
#define CDW_GET_BOOL_WITH_DEFAULT(c, k, d) ((c)->hasKey(k)?(c)->getBoolValue(k):d)
#define CDW_GET_INT32_WITH_DEFAULT(c, k, d) ((c)->hasKey(k)?(c)->getInt32Value(k):d)
#define CDW_GET_INT64_WITH_DEFAULT(c, k, d) ((c)->hasKey(k)?(c)->getInt64Value(k):d)
#define CDW_GET_DOUBLE_WITH_DEFAULT(c, k, d) ((c)->hasKey(k)?(c)->getDoubleValue(k):d)
#define CDW_CHECK_AND_SET(chk, cdw, t, k, v) if(chk){cdw->t(k,v);}
#define CDW_GET_VALUE_WITH_DEFAULT(c, k, t, d) ((c)->hasKey(k)?(c)->t(k):d)

            typedef ChaosUniquePtr<chaos::common::data::CDataWrapper> CDWUniquePtr;
            typedef ChaosSharedPtr<chaos::common::data::CDataWrapper> CDWShrdPtr;
            CHAOS_DEFINE_VECTOR_FOR_TYPE(CDWShrdPtr, VectorCDWShrdPtr);
            CHAOS_DEFINE_VECTOR_FOR_TYPE(CDWUniquePtr, VectorCDWUniquePtr);
#define CreateNewDataWrapper(n,p) CreateNewUniquePtr(chaos::common::data::CDataWrapper, n, p)
            
            
            typedef std::pair<std::string, CDWShrdPtr> PairStrCDWShrdPtr;
            CHAOS_DEFINE_VECTOR_FOR_TYPE(PairStrCDWShrdPtr, VectorStrCDWShrdPtr);
        }
    }
}
#endif


#if defined(__GNUC__) && (__GNUC__ >= 6) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
