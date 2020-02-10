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

#include "PosixFile.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>

#define INFO INFO_LOG(PosixFile)
#define DBG DBG_LOG(PosixFile)
#define ERR ERR_LOG(PosixFile)
#include <chaos/common/utility/TimingUtil.h>
using namespace chaos::metadata_service::object_storage;
using namespace boost::filesystem;
#define MAX_PATH_LEN 512
#if CHAOS_PROMETHEUS
using namespace chaos::common::metric;
#endif
using namespace chaos::common::async_central;

namespace chaos {
namespace metadata_service {
namespace object_storage {

PosixFile::PosixFile(const std::string& name)
    : basedatapath(name) {

        #if CHAOS_PROMETHEUS
    MetricManager::getInstance()->createCounterFamily("mds_storage_io_data", "Measure the data rate for the data sent and read from object storage [byte]");
    counter_write_data_uptr = MetricManager::getInstance()->getNewCounterFromFamily("mds_storage_io_data", {{"type","write_byte"}});
    counter_read_data_uptr = MetricManager::getInstance()->getNewCounterFromFamily("mds_storage_io_data", {{"type","read_byte"}});
    
    MetricManager::getInstance()->createGaugeFamily("mds_storage_op_time", "Measure the time spent by object storageto complete operation [milliseconds]");
    gauge_insert_time_uptr = MetricManager::getInstance()->getNewGaugeFromFamily("mds_storage_op_time", {{"type","insert_time"}});
    gauge_query_time_uptr = MetricManager::getInstance()->getNewGaugeFromFamily("mds_storage_op_time", {{"type","query_time"}});
    DBG<<" CREATED METRICS";

#endif
    DBG<<" BASED DIR:"<<name;
    AsyncCentralManager::getInstance()->addTimer(this, 5000, 5000);

    }

PosixFile::~PosixFile() {}
std::map<std::string,uint64_t> PosixFile::s_lastDirs;
PosixFile::cacheRead_t PosixFile::s_lastAccessedDir;
//ChaosSharedMutex PosixFile::devio_mutex;

void PosixFile::calcFileDir(const std::string& prefix, const std::string& cu,const std::string& tag, uint64_t ts_ms, uint64_t seq, uint64_t runid, char* dir, char* fname) {
  // std::size_t found = cu.find_last_of("/");
  //time_t     t     = (ts_ms==0)?time(NULL):(ts_ms / 1000);
  time_t     t     = (ts_ms / 1000);
  struct tm tinfo;
  localtime_r(&t,&tinfo);
  
  // CU PATH NAME/<yyyy>/<mm>/<dd>/<hour>
  if(tag.size()>0){
    snprintf(dir, MAX_PATH_LEN, "%s/%s/%s", prefix.c_str(), cu.c_str(), tag.c_str());

  } else {
    snprintf(dir, MAX_PATH_LEN, "%s/%s/%.4d/%.2d/%.2d/%.2d", prefix.c_str(), cu.c_str(), tinfo.tm_year + 1900, tinfo.tm_mon+1, tinfo.tm_mday, tinfo.tm_hour);
  }
  // timestamp_runid_seq_ssss
  snprintf(fname, MAX_PATH_LEN, "%llu_%llu_%.10llu", t, runid, seq);
}

int PosixFile::pushObject(const std::string&                       key,
                          const ChaosStringSetConstSPtr            meta_tags,
                          const chaos::common::data::CDataWrapper& stored_object) {
  if (!stored_object.hasKey(chaos::DataPackCommonKey::DPCK_DEVICE_ID) ||
      !stored_object.hasKey(chaos::ControlUnitDatapackCommonKey::RUN_ID) ||
      !stored_object.hasKey(chaos::DataPackCommonKey::DPCK_SEQ_ID)) {
    ERR << CHAOS_FORMAT("Object to store doesn't has the default key!\n %1%", % stored_object.getJSONString());
    return -1;
  }
  const uint64_t ts = chaos::common::utility::TimingUtil::getTimeStamp();
  char     dir[MAX_PATH_LEN];
  char     f[MAX_PATH_LEN];
  int64_t seq,runid;
  std::string tag;
  if(meta_tags->size()>0){
    //tag=std::accumulate(meta_tags->begin(),meta_tags->end(),std::string("_"));
    tag = boost::algorithm::join(*meta_tags.get(),"_");

  }
  seq=stored_object.getInt64Value(chaos::DataPackCommonKey::DPCK_SEQ_ID);
  runid=stored_object.getInt64Value(chaos::ControlUnitDatapackCommonKey::RUN_ID);
  calcFileDir(basedatapath, key,tag, ts,seq , runid, dir, f);
  std::map<std::string,uint64_t>::iterator id=s_lastDirs.find(dir);
  if((id==s_lastDirs.end())||(ts-id->second)>1000){
    boost::filesystem::path p(dir);
      if ((boost::filesystem::exists(p) == false)){
          if((boost::filesystem::create_directories(p) == false)){
            ERR << "cannot create directory:" << p;
            return -1;
          } else {
              DBG<<" CREATED DIR:"<<p;
          }
      }
    s_lastDirs[dir]=ts;
  }
   

  char fname[MAX_PATH_LEN*2];
  snprintf(fname,sizeof(fname),"%s/%s",dir,f);

  ofstream fil(fname, std::ofstream::binary);
  //DBG << "["<<ts<<"] WRITE \"" << dir << "\" seq:" << seq << " runid:" << runid << " data size:" << stored_object.getBSONRawSize();
  if(fil.is_open()){
    fil.write((const char*)stored_object.getBSONRawData(), stored_object.getBSONRawSize());
#if CHAOS_PROMETHEUS
   
        (*counter_write_data_uptr) += stored_object.getBSONRawSize();
#endif
    fil.close();
  } else {
      ERR<<" CANNOT WRITE:"<<fname;
      return -2;
      }
#if CHAOS_PROMETHEUS

  (*gauge_insert_time_uptr)=(chaos::common::utility::TimingUtil::getTimeStamp()-ts);
#endif
  return 0;
}

//!Retrieve an object from the object persistence layer
int PosixFile::getObject(const std::string&               key,
                         const uint64_t&                  timestamp,
                         chaos::common::data::CDWShrdPtr& object_ptr_ref) {
  ERR << " NOT IMPLEMENTED";
  return -1;
}

//!Retrieve the last inserted object from the object persistence layer
int PosixFile::getLastObject(const std::string&               key,
                             chaos::common::data::CDWShrdPtr& object_ptr_ref) {
                                   ERR << " NOT IMPLEMENTED";

                                 return -1;
}

//!delete objects that are contained between intervall (exstreme included)
int PosixFile::deleteObject(const std::string& key,
                            uint64_t           start_timestamp,
                            uint64_t           end_timestamp) {
  uint64_t start_aligned=start_timestamp - (start_timestamp%(3600*1000));
  char     dir[MAX_PATH_LEN];
  char     f[MAX_PATH_LEN];
  
  DBG<<"Searching \""<<key<<"\" from: "<<chaos::common::utility::TimingUtil::toString(start_timestamp)<<" to:"<<chaos::common::utility::TimingUtil::toString(end_timestamp);
  for (uint64_t start = start_aligned; start < end_timestamp; start += (3600*1000)) {
        calcFileDir(basedatapath, key,"", start, 0, 0, dir, f);
        boost::filesystem::path p(dir);
        if (boost::filesystem::exists(p) && is_directory(p)) {
          INFO<<"REMOVING "<<p;
          remove_all(p);
          std::map<std::string,uint64_t>::iterator id=s_lastDirs.find(dir);
          if(id!=s_lastDirs.end()){
            s_lastDirs.erase(id);
          }

        }
  }
    
  return 0;
}
struct path_leaf_string
{
    std::string operator()(const boost::filesystem::directory_entry& entry) const
    {
        return entry.path().leaf().string();
    }
};
uint32_t PosixFile::countFromPath(boost::filesystem::path& p,const uint64_t timestamp_from,
                          const uint64_t timestamp_to){
    uint32_t elements=0;
    if (boost::filesystem::exists(p) && is_directory(p)) {
      std::vector<path> v;
      std::transform(directory_iterator(p), directory_iterator(), std::back_inserter(v), path_leaf_string());
      uint64_t iseq, irunid, tim;

      for (std::vector<path>::iterator it = v.begin(); it != v.end(); it++) {
        sscanf(it->c_str(), "%Lu_%llu_%llu", &tim, &irunid, &iseq);
        tim *= 1000;
        if ((tim < timestamp_to) && (tim >= timestamp_from)) {
          elements++;
      }

    }
    }
    return elements;
}

int PosixFile::getFromPath(const std::string& dir,const uint64_t timestamp_from,
                          const uint64_t timestamp_to,
                          const uint32_t page_len,
                          abstraction::VectorObject& found_object_page,
                          chaos::common::direct_io::channel::opcode_headers::SearchSequence& last_record_found_seq){
  uint64_t seqid    = last_record_found_seq.datapack_counter;
  uint64_t runid    = last_record_found_seq.run_id;
  int elements=0;
      cacheRead_t::iterator di=s_lastAccessedDir.find(dir);
      if(di==s_lastAccessedDir.end()){
        boost::filesystem::path p(dir);

       if (boost::filesystem::exists(p) && is_directory(p)) {
           ChaosWriteLock ll(s_lastAccessedDir[dir].devio_mutex);

           std::transform(directory_iterator(p), directory_iterator(), std::back_inserter(s_lastAccessedDir[dir].sorted_path), path_leaf_string());

         // std::copy(directory_iterator(cdir), directory_iterator(), std::back_inserter(v));
          std::sort(s_lastAccessedDir[dir].sorted_path.begin(), s_lastAccessedDir[dir].sorted_path.end());
          s_lastAccessedDir[dir].ts=chaos::common::utility::TimingUtil::getTimeStamp();
       }
       }
        for (std::vector<std::string>::iterator it = s_lastAccessedDir[dir].sorted_path.begin(); it != s_lastAccessedDir[dir].sorted_path.end(); it++) {
            uint64_t iseq, irunid, tim;
          
            sscanf(it->c_str(), "%Lu_%llu_%llu", &tim, &irunid, &iseq);
            tim *= 1000;
            if ((tim < timestamp_to) && (tim >= timestamp_from) && (irunid >= runid) && iseq >= seqid) {
              char tmpbuf[MAX_PATH_LEN];
              snprintf(tmpbuf,sizeof(tmpbuf),"%s/%s",dir.c_str(),it->c_str());
              ifstream infile(tmpbuf, std::ofstream::binary);
              infile.seekg(0, infile.end);
              long size = infile.tellg();
              infile.seekg(0);
              char* buffer = new char[size];
              infile.read(buffer, size);
#if CHAOS_PROMETHEUS
   
        (*counter_read_data_uptr) += size;
#endif

            //  DBG << "retriving \"" << *it << "\" seq:" << iseq << " runid:" << irunid << " data size:" << size;
              chaos::common::data::CDWShrdPtr new_obj(new chaos::common::data::CDataWrapper((const char*)buffer, size));

              found_object_page.push_back(new_obj);
              delete buffer;
              infile.close();
              last_record_found_seq.run_id           = irunid;
              last_record_found_seq.datapack_counter = iseq;
              elements++;
              if(page_len>0 && (elements>=page_len)){
                return elements;
              }
            }
          }
        
        return elements;
 
}
//!search object into object persistence layer
int PosixFile::findObject(const std::string&                                                 key,
                          const ChaosStringSet&                                              meta_tags,
                          const ChaosStringSet&                                              projection_keys,
                          const uint64_t                                                     timestamp_from,
                          const uint64_t                                                     timestamp_to,
                          const uint32_t                                                     page_len,
                          abstraction::VectorObject&                                                      found_object_page,
                          chaos::common::direct_io::channel::opcode_headers::SearchSequence& last_record_found_seq) {
  int err = 0;

  uint64_t seqid    = last_record_found_seq.datapack_counter;
  uint64_t runid    = last_record_found_seq.run_id;
  int      old_hour = -1;
  char     dir[MAX_PATH_LEN];
  char     f[MAX_PATH_LEN];
  dir[0] = 0;
    const uint64_t ts = chaos::common::utility::TimingUtil::getTimeStamp();

  int elements=0;
  try {
     std::string tag;
     {
      if(meta_tags.size()>0){

        //tag=std::accumulate(meta_tags.begin(),meta_tags.end(),std::string("_"));
        tag = boost::algorithm::join(meta_tags,"_");
      }
      std::string st=chaos::common::utility::TimingUtil::toString(timestamp_from);
      std::string sto=chaos::common::utility::TimingUtil::toString(timestamp_to);
      DBG << "Search "<<key.c_str()<<" from:"<<st<<" to:"<< sto<< " seq:" << seqid << " runid:" << runid << " TAGS:"<<tag;

     }

      if(meta_tags.size()>0){
        calcFileDir(basedatapath, key, tag, timestamp_from, seqid, runid, dir, f);

        elements+=getFromPath(dir,timestamp_from,timestamp_to,page_len,found_object_page,last_record_found_seq);
        
        return 0;

    }
    // align to hour
    uint64_t start_aligned=timestamp_from - (timestamp_from%(3600*1000));
    for (uint64_t start = start_aligned; start < timestamp_to; start += (3600*1000)) {
      time_t     t     = (start / 1000);
      //struct tm* tinfo = localtime(&t);
      struct tm tinfo;
       localtime_r(&t,&tinfo);
      if ((tinfo.tm_hour != old_hour)) {
        calcFileDir(basedatapath, key,tag, start, seqid, runid, dir, f);
       // boost::filesystem::path p(dir);
        DBG << "["<<ctime(&t)<<"] Looking in \"" << dir << "\" seq:" << seqid << " runid:" << runid;

        elements+=getFromPath(dir,timestamp_from,timestamp_to,page_len,found_object_page,last_record_found_seq);
        if(elements>=page_len){
          DBG <<"["<< dir<<"] Found "<<elements<<" page:"<<page_len<< " last runid:"<<last_record_found_seq.run_id<<" last seq:"<<last_record_found_seq.datapack_counter;
#if CHAOS_PROMETHEUS

  (*gauge_query_time_uptr)=(chaos::common::utility::TimingUtil::getTimeStamp()-ts);
#endif
          return 0;
        }
        old_hour = tinfo.tm_hour;
      }
    }

  } catch (const std::exception e) {
    ERR << e.what();
  }
  if(err==0 && elements>0){
#if CHAOS_PROMETHEUS

    (*gauge_query_time_uptr)=(chaos::common::utility::TimingUtil::getTimeStamp()-ts);
#endif
  }
  return err;
}

//!fast search object into object persistence layer
/*!
                     Fast search return only data index to the client, in this csae client ned to use api to return the single
                     or grouped data
                     */
int PosixFile::findObjectIndex(const abstraction::DataSearch&                                                  search,
                               abstraction::VectorObject&                                                      found_object_page,
                               chaos::common::direct_io::channel::opcode_headers::SearchSequence& last_record_found_seq) {
  ERR << " NOT IMPLEMENTED";

  return 0;
}

//! return the object asosciated with the index array
/*!
                     For every index object witl be returned the associated data object, if no data is received will be
                     insert an empty object
                     */
int PosixFile::getObjectByIndex(const chaos::common::data::CDWShrdPtr& index,
                                chaos::common::data::CDWShrdPtr&       found_object) {
  ERR << " NOT IMPLEMENTED";

  return 0;
}
void PosixFile::timeout() {
    uint64_t ts = chaos::common::utility::TimingUtil::getTimeStamp();

    // remove directory write cache
    for(std::map<std::string,uint64_t>::iterator id=s_lastDirs.begin();id!=s_lastDirs.end();id){
      if((ts-id->second)>3600000){
        //not anymore used
        s_lastDirs.erase(id++);
      } else {
        id++;
      }
    }
    for(cacheRead_t::iterator id=s_lastAccessedDir.begin();id!=s_lastAccessedDir.end();id){
      if((ts-id->second.ts)>5000){
        //not anymore used
        if(id->second.devio_mutex.try_lock()){
          s_lastAccessedDir.erase(id++);
        }
      } else {
        id++;
      }
    }

}

//!return the number of object for a determinated key that are store for a time range
int PosixFile::countObject(const std::string& key,
                           const uint64_t     timestamp_from,
                           const uint64_t     timestamp_to,
                          uint64_t&    object_count) {

  int      old_hour = -1;
  char     dir[MAX_PATH_LEN];
  char     f[MAX_PATH_LEN];
  dir[0] = 0;
    object_count=0;
    chaos::common::direct_io::channel::opcode_headers::SearchSequence last_record_found_seq;
    last_record_found_seq.datapack_counter=0;
    last_record_found_seq.run_id=0;
    
    uint64_t start_aligned=timestamp_from - (timestamp_from%(3600*1000));
    for (uint64_t start = start_aligned; start < timestamp_to; start += (3600*1000)) {
      time_t     t     = (start / 1000);
      struct tm tinfo;
       localtime_r(&t,&tinfo);
      if ((tinfo.tm_hour != old_hour)) {
        calcFileDir(basedatapath, key,"", start, 0, 0, dir, f);
        boost::filesystem::path p(dir);
        
        object_count+=countFromPath(p,timestamp_from,timestamp_to);
        old_hour = tinfo.tm_hour;
      }
    }
  return 0;
}

}  // namespace object_storage
}  // namespace metadata_service
}  // namespace chaos
