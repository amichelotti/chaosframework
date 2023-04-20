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

#include "InfluxDB.h"
#include <chaos/common/configuration/GlobalConfiguration.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#define INFO INFO_LOG(InfluxDB)
#define DBG DBG_LOG(InfluxDB)
#define ERR ERR_LOG(InfluxDB)
#include <chaos/common/utility/TimingUtil.h>
using namespace chaos::metadata_service::object_storage;

#if CHAOS_PROMETHEUS
using namespace chaos::common::metric;
#endif
using namespace chaos::common::async_central;

namespace chaos {
namespace metadata_service {
namespace object_storage {
#if CHAOS_PROMETHEUS

/*static global*/
chaos::common::metric::CounterUniquePtr InfluxDB::counter_write_data_uptr;
chaos::common::metric::CounterUniquePtr InfluxDB::counter_read_data_uptr;
chaos::common::metric::GaugeUniquePtr   InfluxDB::gauge_insert_time_uptr;
chaos::common::metric::GaugeUniquePtr   InfluxDB::gauge_query_time_uptr;
#endif

std::stringstream InfluxDB::measurements;
uint32_t          InfluxDB::nmeas;

/**************/

InfluxDB::InfluxDB(const influxdb_cpp::server_info& serverinfo)
    : si(serverinfo), last_insert(0) {
  nmeas       = 0;
  last_insert = chaos::common::utility::TimingUtil::getTimeStamp();

  // AsyncCentralManager::getInstance()->addTimer(this, 1000, 1000);
  push_th = boost::thread(&InfluxDB::push_process, this);
}

InfluxDB::~InfluxDB() {
  push_end = true;
  push_th.join();
}
inline bool skipDefault(const std::string& name) {
  if (name == chaos::DataPackCommonKey::DPCK_DATASET_TYPE) return true;
  if (name == chaos::DataPackCommonKey::DPCK_DEVICE_ID) return true;
  if (name == chaos::DataServiceNodeDefinitionKey::DS_STORAGE_TYPE) return true;
  if (name == chaos::DataPackCommonKey::NODE_MDS_TIMEDIFF) return true;
  if (name == chaos::ControlUnitDatapackCommonKey::RUN_ID) return true;
  if (name == chaos::DataPackCommonKey::DPCK_DEVICE_ID) return true;
  if (name == chaos::NodeHealtDefinitionKey::NODE_HEALT_MDS_TIMESTAMP) return true;
  if (name == chaos::DataPackCommonKey::DPCK_SEQ_ID) return true;
  if (name == chaos::DataPackCommonKey::DPCK_TIMESTAMP) return true;

  return false;
}
int InfluxDB::pushObject(const std::string&                       key,
                         const ChaosStringSetConstSPtr            meta_tags,
                         const chaos::common::data::CDataWrapper& stored_object) {
  if (!stored_object.hasKey(chaos::DataPackCommonKey::DPCK_DEVICE_ID) ||
      !stored_object.hasKey(chaos::ControlUnitDatapackCommonKey::RUN_ID) ||
      !stored_object.hasKey(chaos::DataPackCommonKey::DPCK_SEQ_ID)) {
    ERR << CHAOS_FORMAT("Object to store doesn't has the default key!\n %1%", % stored_object.getJSONString());
    return -1;
  }
  const uint64_t now = chaos::common::utility::TimingUtil::getTimeStamp();

  //
  const int64_t ts = stored_object.getInt64Value(chaos::DataPackCommonKey::DPCK_TIMESTAMP);  // TimingUtil::getTimeStamp() & 0xFFFFFFFFFFFFFF00ULL;

  uint8_t*    buf;
  size_t      buflen;
  int64_t     seq, runid;
  std::string tag;
static const unsigned int DPCK_DATASET_TYPE_INPUT = 1;
//! the constant that represent the custom dataset type
static const unsigned int DPCK_DATASET_TYPE_CUSTOM = 2;
//! the constant that represent the system dataset type
static const unsigned int DPCK_DATASET_TYPE_SYSTEM = 3;
//! the constant that represent the health dataset type
static const unsigned int DPCK_DATASET_TYPE_HEALTH = 4;
//! the constant that represent the alarm dataset type
static const unsigned int DPCK_DATASET_TYPE_DEV_ALARM = 5;
//! the constant that represent the alarm dataset type
static const unsigned int DPCK_DATASET_TYPE_CU_ALARM = 6;
//! the last log dataset
static const unsigned int DPCK_DATASET_TYPE_LOG = 7;
  int typ=0;
  if(stored_object.hasKey(DataPackCommonKey::DPCK_DATASET_TYPE)){
    typ =stored_object.getInt32Value(DataPackCommonKey::DPCK_DATASET_TYPE);
  }

  ChaosStringVector contained_key;
  stored_object.getAllKey(contained_key);

  ChaosLockGuard ll(iolock);
  measurements << stored_object.getStringValue(chaos::DataPackCommonKey::DPCK_DEVICE_ID);
  std::string pref;

  switch(typ){
    case DPCK_DATASET_TYPE_INPUT:
      pref="inp.";
      break;
    case DPCK_DATASET_TYPE_CU_ALARM:
    case DPCK_DATASET_TYPE_DEV_ALARM:
      pref="alm.";
      break;
    case DPCK_DATASET_TYPE_SYSTEM:
      pref="sys.";
      break;
    case DPCK_DATASET_TYPE_CUSTOM:
      pref="cst.";
      break;
    default:
      break;

  }
  
  if ((meta_tags.get()) && (meta_tags->size() > 0)) {
    // tag=std::accumulate(meta_tags->begin(),meta_tags->end(),std::string("_"));
    measurements << ",tag=" << *(meta_tags->begin());
  }
  int first = 0;
  for (std::vector<std::string>::iterator i = contained_key.begin(); i != contained_key.end(); i++) {
    if (!skipDefault(*i)) {
      char c = (first == 0) ? ' ' : ',';
      switch (stored_object.getValueType(*i)) {
        case DataType::TYPE_BOOLEAN:
          measurements << c << pref<<*i << "=" << ((stored_object.getBoolValue(*i)) ? 't' : 'f');
          nmeas++;
          first++;
          break;
        case DataType::TYPE_INT32:
        case DataType::TYPE_INT64:
        case DataType::TYPE_UINT64:

          measurements << c <<pref<< *i << "=" << stored_object.getStringValue(*i) << 'i';
          nmeas++;
          first++;

          break;
        case DataType::TYPE_DOUBLE: {
          double d = stored_object.getDoubleValue(*i);
          if (std::isfinite(d)) {
            measurements << c << pref<<*i << "=" << d;
            nmeas++;
            first++;
          }

          break;
        }
        case DataType::TYPE_STRING: {
          std::string val = stored_object.getStringValue(*i);
          if ((val.size() > 0) && (val.size() < (1024 * 64))) {
            measurements << c << pref<<*i << "=\"" << val << "\"";
            first++;
            nmeas++;
          }
          break;
        }
        case DataType::TYPE_VECTOR_BOOL: {
          uint32_t    size = 0;
          const bool* ptr  = (const bool*)stored_object.getBinaryValue(*i, size);
          
          if (ptr && ( size / sizeof(bool)< si.max_array_size)) {
            for (int cnt = 0; (cnt <size/sizeof(bool)) ; cnt++) {
              measurements << c <<pref<< *i + "." << cnt << "=" << ((ptr[cnt]) ? 't' : 'f');
              first++;
              nmeas++;
            }
          } else {
              ERR << "Skipped vector (bool) measurement \"" << *i << "\", size " <<size / sizeof(bool) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_INT32: {
          uint32_t       size = 0;
          const int32_t* ptr  = (const int32_t*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(int32_t)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(int32_t)); cnt++) {
              measurements << c << pref<<*i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          } else {
              ERR << "Skipped vector (int32_t) measurement \"" << *i << "\", size " <<size / sizeof(int32_t) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_UINT32: {
          uint32_t        size = 0;
          const uint32_t* ptr  = (const uint32_t*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(int32_t)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(int32_t)) ; cnt++) {
              measurements << c <<pref<< *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          } else {
              ERR << "Skipped vector (uint32_t) measurement \"" << *i << "\", size " <<size / sizeof(uint32_t) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_UINT8: {
          uint32_t       size = 0;
          const uint8_t* ptr  = (const uint8_t*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(uint8_t)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(uint8_t)); cnt++) {
              measurements << c <<pref<< *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          } else {
              ERR << "Skipped vector (uint8_t) measurement \"" << *i << "\", size " <<size / sizeof(uint8_t) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_UINT16: {
          uint32_t        size = 0;
          const uint16_t* ptr  = (const uint16_t*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(uint16_t)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(uint16_t)) ; cnt++) {
              measurements << c << pref<<*i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          } else {
              ERR << "Skipped vector (uint16_t) measurement \"" << *i << "\", size " <<size / sizeof(uint16_t) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_INT16: {
          uint32_t       size = 0;
          const int16_t* ptr  = (const int16_t*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(int16_t)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(int16_t)) ; cnt++) {
              measurements << c <<pref<< *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          } else {
              ERR << "Skipped vector (int16_t) measurement \"" << *i << "\", size " <<size / sizeof(int16_t) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_INT8: {
          uint32_t      size = 0;
          const int8_t* ptr  = (const int8_t*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(int8_t)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(int8_t)); cnt++) {
              measurements << c << pref<<*i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          } else {
              ERR << "Skipped vector (int8_t) measurement \"" << *i << "\", size " <<size / sizeof(int8_t) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_INT64: {
          uint32_t       size = 0;
          const int64_t* ptr  = (const int64_t*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(int64_t)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(int64_t)); cnt++) {
              measurements << c << pref<<*i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          }else {
              ERR << "Skipped vector (int64) measurement \"" << *i << "\", size " <<size / sizeof(int64_t) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_DOUBLE: {
          uint32_t      size = 0;
          const double* ptr  = (const double*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(double)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(double)); cnt++) {
              if (std::isfinite(ptr[cnt])) {
                measurements << c << pref<<*i + "." << cnt << "=" << ptr[cnt];
                first++;
                nmeas++;
              }
              //  DBG<< c << *i+"_" <<cnt<< "=" << ptr[cnt];
            }
          } else {
              ERR << "Skipped vector (double) measurement \"" << *i << "\", size " <<size / sizeof(double) << " max " << si.max_array_size;

          }
          break;
        }
        case DataType::TYPE_VECTOR_FLOAT: {
          uint32_t     size = 0;
          const float* ptr  = (const float*)stored_object.getBinaryValue(*i, size);
          if (ptr&& ( size / sizeof(float)< si.max_array_size)) {
            for (int cnt = 0; (cnt < size / sizeof(float)) ; cnt++) {
              if (std::isfinite(ptr[cnt])) {
                measurements << c << pref<<*i + "." << cnt << "=" << ptr[cnt];
                first++;
                nmeas++;
              }
              //  DBG<< c << *i+"_" <<cnt<< "=" << ptr[cnt];
            }
          } else {
              ERR << "Skipped vector (float) measurement \"" << *i << "\", size " <<size / sizeof(float) << " max " << si.max_array_size;

          }
          break;
        }

        default:
          //       DBG<<*i<<" packet cannot be influxed, type:"<<stored_object.getValueType(*i);
          break;

          // not handled
          //   l(meas,influxdb::api::key_value_pairs(*i,stored_object.getStringValue(*i)));
          //   break;
      }
    }
    if ((i + 1) == contained_key.end()) {
      measurements << " " << ts << "\n";
    }
  }

#if CHAOS_PROMETHEUS

  (*counter_write_data_uptr) += stored_object.getBSONRawSize();

#endif

#if CHAOS_PROMETHEUS

  (*gauge_insert_time_uptr) = (chaos::common::utility::TimingUtil::getTimeStamp() - ts);
#endif

  return 0;
}

//! Retrieve an object from the object persistence layer
int InfluxDB::getObject(const std::string&               key,
                        const uint64_t&                  timestamp,
                        chaos::common::data::CDWShrdPtr& object_ptr_ref) {
  ERR << " NOT IMPLEMENTED";
  return -1;
}

//! Retrieve the last inserted object from the object persistence layer
int InfluxDB::getLastObject(const std::string&               key,
                            chaos::common::data::CDWShrdPtr& object_ptr_ref) {
  ERR << " NOT IMPLEMENTED";

  return -1;
}

//! delete objects that are contained between intervall (exstreme included)
int InfluxDB::deleteObject(const std::string& key,
                           uint64_t           start_timestamp,
                           uint64_t           end_timestamp) {
  return 0;
}

//! search object into object persistence layer
int InfluxDB::findObject(const std::string&                                                 key,
                         const ChaosStringSet&                                              meta_tags,
                         const ChaosStringSet&                                              projection_keys,
                         const uint64_t                                                     timestamp_from,
                         const uint64_t                                                     timestamp_to,
                         const uint32_t                                                     page_len,
                         abstraction::VectorObject&                                         found_object_page,
                         chaos::common::direct_io::channel::opcode_headers::SearchSequence& last_record_found_seq) {
  int err = 0;

  uint64_t last_ts=0;
  last_record_found_seq.run_id=0;
  
  std::stringstream ss;
  ss<<"SELECT ";
  if(projection_keys.size()==0){
    ss<<"*";
  } else {
    for(ChaosStringSet::iterator i = projection_keys.begin();i!=projection_keys.end();i++){
      ss<<*i;
      if((++i)!=projection_keys.end()){
        ss<<",";
      }
      --i;
    }
  }
 //std::stringstream stype;stype<<"SHOW FIELD KEYS FROM "<< "\""<<key<<"\"";

  ss<<" FROM \""<<key<<"\" WHERE time>="<<timestamp_from*1000000<<" AND time<"<<timestamp_to*1000000;
  if(meta_tags.size()){
    ss<<" AND \"tag\"='"<<*meta_tags.begin()<<"'";
  }
  if(page_len>0){
    ss<<" LIMIT "<<page_len;
  }

  std::string resp;
  int ret=influxdb_cpp::query(resp,ss.str(),si);
  //DBG<<ss.str()<<" returned "<<ret<<" ->"<<resp;

  /*if(ret==0){

  ret=influxdb_cpp::query(resp,ss.str(),si);
  chaos::common::data::CDataWrapper data;
  data.setSerializedJsonData(resp.c_str());
  if(data.hasKey("results")&& data.isVectorValue("results")){
     chaos::common::data::CMultiTypeDataArrayWrapperSPtr results=data.getVectorValue("results");
     chaos::common::data::CDWUniquePtr serie=results->getCDataWrapperElementAtIndex(0);
     if(serie.get()&&serie->hasKey("series")&&serie->isVectorValue("series")){
       chaos::common::data::CDWUniquePtr cu=cud->getCDataWrapperElementAtIndex(0);
            if(cu.get()&&cu->hasKey("values")&&){
              std::string name=cu->getStringValue("name");
              if(cu->hasKey("columns")&&cu->isVectorValue("columns")){

              }

            }
     }
  }*/

    
  if(ret==0){
    chaos::common::data::CDataWrapper data;
    data.setSerializedJsonData(resp.c_str());
    //DBG<<data.getJSONString();

    if(data.hasKey("results")&& data.isVectorValue("results")){
     chaos::common::data::CMultiTypeDataArrayWrapperSPtr results=data.getVectorValue("results");
    for(int cnt_res=0;cnt_res<results->size();cnt_res++){

     chaos::common::data::CDWUniquePtr serie=results->getCDataWrapperElementAtIndex(cnt_res);
     if(serie.get()&&serie->hasKey("series")&&serie->isVectorValue("series")){
           chaos::common::data::CMultiTypeDataArrayWrapperSPtr cud=serie->getVectorValue("series");
           for(int cnt_series=0;cnt_series<cud->size();cnt_series++){
           chaos::common::data::CDWUniquePtr cu=cud->getCDataWrapperElementAtIndex(cnt_series);
            if(cu.get()&&cu->hasKey("name")){
              std::string name=cu->getStringValue("name");
              std::vector<std::string> cols;
              if(cu->hasKey("columns")&&cu->isVectorValue("columns")){
                chaos::common::data::CMultiTypeDataArrayWrapperSPtr col=cu->getVectorValue("columns");
                cols=(std::vector<std::string>)*col;
              }
              if(cu->hasKey("values")&&cu->isVectorValue("values")){
                chaos::common::data::CMultiTypeDataArrayWrapperSPtr vals=cu->getVectorValue("values");
                for(int cnt=0;cnt<vals->size();cnt++){
                    chaos::common::data::CDWShrdPtr dd(new chaos::common::data::CDataWrapper());
                    if(vals->isArrayElementAtIndex(cnt)){
                        chaos::common::data::CMultiTypeDataArrayWrapperSPtr val=vals->getVectorElementAtIndex(cnt);
                        for(int cntt=0;cntt<val->size();cntt++){
                          if(cols[cntt]=="time"){
                            int64_t ts=chaos::common::utility::TimingUtil::getTimestampFromString(val->getStringElementAtIndex(cntt),"%Y-%m-%dT%H:%M:%S%fZ");
                            dd->append(chaos::DataPackCommonKey::DPCK_DEVICE_ID,key);
                            dd->append(chaos::DataPackCommonKey::DPCK_TIMESTAMP,ts);
                            if(ts>last_ts) last_ts=ts;
                          }
                          dd->append(cols[cntt],val->getBSONElementAtIndex(cntt));
                        }
                   // DBG<<cnt<<"] "<<dd->getCompliantJSONString();
   
                    found_object_page.push_back(dd);
                    last_record_found_seq.datapack_counter++;
                    last_record_found_seq.ts=last_ts;
                    }
                  
                }
              }

            }
           }
     }

     }
    }
  }
  

#if CHAOS_PROMETHEUS

  // (*gauge_query_time_uptr) = (chaos::common::utility::TimingUtil::getTimeStamp() - ts);
#endif

  return ret;
}

//! fast search object into object persistence layer
/*!
                     Fast search return only data index to the client, in this csae client ned to use api to return the single
                     or grouped data
                     */
int InfluxDB::findObjectIndex(const abstraction::DataSearch&                                     search,
                              abstraction::VectorObject&                                         found_object_page,
                              chaos::common::direct_io::channel::opcode_headers::SearchSequence& last_record_found_seq) {
  ERR << " NOT IMPLEMENTED";

  return 0;
}

//! return the object asosciated with the index array
/*!
                     For every index object witl be returned the associated data object, if no data is received will be
                     insert an empty object
                     */
int InfluxDB::getObjectByIndex(const chaos::common::data::CDWShrdPtr& index,
                               chaos::common::data::CDWShrdPtr&       found_object) {
  ERR << " NOT IMPLEMENTED";

  return 0;
}

void InfluxDB::push_process() {
  push_end = false;
  while (push_end == false) {
    const uint64_t now = chaos::common::utility::TimingUtil::getTimeStamp();

    if ((nmeas >= si.max_mesurements) || ((nmeas > 0) && ((now - last_insert) > si.max_time_ms))) {
      ChaosLockGuard ll(iolock);
      std::string sret;
       int ret=influxdb_cpp::push_db( sret, measurements.str(), si);

      //int ret = influxdb_cpp::detail::inner::http_request("POST", "write", "", measurements.str(), si, NULL);
      if (ret == 0) {
        if((nmeas >= si.max_mesurements)){
          DBG << "exeeded measurements "<<si.max_mesurements<<" sending " << nmeas << " measurements, " << measurements.str().size() << " bytes, " << nmeas * 1000 / (now - last_insert) << " mesure/s";
        }
      } else {
        ERR << "Error sending " << nmeas << " measurements, " << measurements.str().size() << " bytes, " << nmeas * 1000 / (now - last_insert) << " mesure/s, error code:" << ret;
      }
      last_insert = now;

      nmeas = 0;
      measurements.clear();
      measurements.str("");
    }

    usleep(1000*si.poll_time_ms);
  }

}

//! return the number of object for a determinated key that are store for a time range
int InfluxDB::countObject(const std::string& key,
                          const uint64_t     timestamp_from,
                          const uint64_t     timestamp_to,
                          uint64_t&          object_count) {

  std::stringstream ss;
  ss<<"SELECT COUNT(*) FROM \""<<key<<"\" WHERE time>="<<timestamp_from*1000000<<" AND time<"<<timestamp_to*1000000;
  std::string resp;
  int ret=influxdb_cpp::query(resp,ss.str(),si);                        
  if(ret==0){
   // DBG<<"COUNT:"<<resp;

    chaos::common::data::CDataWrapper data;
    data.setSerializedJsonData(resp.c_str());
    //DBG<<data.getJSONString();

    if(data.hasKey("results")&& data.isVectorValue("results")){
     chaos::common::data::CMultiTypeDataArrayWrapperSPtr results=data.getVectorValue("results");
    for(int cnt_res=0;cnt_res<results->size();cnt_res++){

     chaos::common::data::CDWUniquePtr serie=results->getCDataWrapperElementAtIndex(cnt_res);
     if(serie.get()&&serie->hasKey("series")&&serie->isVectorValue("series")){
           chaos::common::data::CMultiTypeDataArrayWrapperSPtr cud=serie->getVectorValue("series");
           for(int cnt_series=0;cnt_series<cud->size();cnt_series++){
           chaos::common::data::CDWUniquePtr cu=cud->getCDataWrapperElementAtIndex(cnt_series);
              if(cu->hasKey("values")&&cu->isVectorValue("values")){
                chaos::common::data::CMultiTypeDataArrayWrapperSPtr vals=cu->getVectorValue("values");
                object_count=0;

                 
                

                if(vals->size()>0){
                    chaos::common::data::CMultiTypeDataArrayWrapperSPtr val=vals->getVectorElementAtIndex(0);
                    if(val->size()>1){
                     // DBG<<"size: "<<vals->size();
                      object_count=val->getInt32ElementAtIndex(val->size()-1);

                    }
                  // 0 is time
                }
              }}}}}
  }
  return ret;
}

}  // namespace object_storage
}  // namespace metadata_service
}  // namespace chaos
