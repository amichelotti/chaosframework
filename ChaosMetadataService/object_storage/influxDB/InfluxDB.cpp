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
#include <boost/regex.hpp>
#define INFO INFO_LOG(InfluxDB)
#define DBG DBG_LOG(InfluxDB)
#define ERR ERR_LOG(InfluxDB)
#include <chaos/common/utility/TimingUtil.h>
#define MAX_ARRAY_POINTS 512
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

  std::string meas = stored_object.getStringValue(chaos::DataPackCommonKey::DPCK_DEVICE_ID);

  ChaosStringVector contained_key;
  stored_object.getAllKey(contained_key);

  ChaosLockGuard ll(iolock);

  measurements << stored_object.getStringValue(chaos::DataPackCommonKey::DPCK_DEVICE_ID);
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
          measurements << c << *i << "=" << ((stored_object.getBoolValue(*i)) ? 't' : 'f');
          nmeas++;
          first++;
          break;
        case DataType::TYPE_INT32:
        case DataType::TYPE_INT64:
        case DataType::TYPE_UINT64:

          measurements << c << *i << "=" << stored_object.getStringValue(*i) << 'i';
          nmeas++;
          first++;

          break;
        case DataType::TYPE_DOUBLE: {
          double d = stored_object.getDoubleValue(*i);
          if (std::isfinite(d)) {
            measurements << c << *i << "=" << d;
            nmeas++;
            first++;
          }

          break;
        }
        case DataType::TYPE_STRING: {
          std::string val = stored_object.getStringValue(*i);
          if ((val.size() > 0) && (val.size() < (1024 * 64))) {
            measurements << c << *i << "=\"" << val << "\"";
            first++;
            nmeas++;
          }
          break;
        }
        case DataType::TYPE_VECTOR_BOOL: {
          uint32_t    size = 0;
          const bool* ptr  = (const bool*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(bool)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              measurements << c << *i + "." << cnt << "=" << ((ptr[cnt]) ? 't' : 'f');
              first++;
              nmeas++;
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_INT32: {
          uint32_t       size = 0;
          const int32_t* ptr  = (const int32_t*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(int32_t)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              measurements << c << *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_UINT32: {
          uint32_t        size = 0;
          const uint32_t* ptr  = (const uint32_t*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(int32_t)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              measurements << c << *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_UINT8: {
          uint32_t       size = 0;
          const uint8_t* ptr  = (const uint8_t*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(uint8_t)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              measurements << c << *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_UINT16: {
          uint32_t        size = 0;
          const uint16_t* ptr  = (const uint16_t*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(uint16_t)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              measurements << c << *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_INT16: {
          uint32_t       size = 0;
          const int16_t* ptr  = (const int16_t*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(int16_t)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              measurements << c << *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_INT8: {
          uint32_t      size = 0;
          const int8_t* ptr  = (const int8_t*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(int8_t)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              measurements << c << *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_INT64: {
          uint32_t       size = 0;
          const int64_t* ptr  = (const int64_t*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(int64_t)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              measurements << c << *i + "." << cnt << "=" << ptr[cnt] << "i";
              first++;
              nmeas++;
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_DOUBLE: {
          uint32_t      size = 0;
          const double* ptr  = (const double*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(double)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              if (std::isfinite(ptr[cnt])) {
                measurements << c << *i + "." << cnt << "=" << ptr[cnt];
                first++;
                nmeas++;
              }
              //  DBG<< c << *i+"_" <<cnt<< "=" << ptr[cnt];
            }
          }
          break;
        }
        case DataType::TYPE_VECTOR_FLOAT: {
          uint32_t     size = 0;
          const float* ptr  = (const float*)stored_object.getBinaryValue(*i, size);
          if (ptr) {
            for (int cnt = 0; (cnt < size / sizeof(float)) && (cnt < MAX_ARRAY_POINTS); cnt++) {
              if (std::isfinite(ptr[cnt])) {
                measurements << c << *i + "." << cnt << "=" << ptr[cnt];
                first++;
                nmeas++;
              }
              //  DBG<< c << *i+"_" <<cnt<< "=" << ptr[cnt];
            }
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

  uint64_t seqid = last_record_found_seq.datapack_counter;
  uint64_t runid = last_record_found_seq.run_id;

#if CHAOS_PROMETHEUS

  // (*gauge_query_time_uptr) = (chaos::common::utility::TimingUtil::getTimeStamp() - ts);
#endif

  return err;
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

    sleep(1);
  }

}

//! return the number of object for a determinated key that are store for a time range
int InfluxDB::countObject(const std::string& key,
                          const uint64_t     timestamp_from,
                          const uint64_t     timestamp_to,
                          uint64_t&          object_count) {
  return 0;
}

}  // namespace object_storage
}  // namespace metadata_service
}  // namespace chaos
