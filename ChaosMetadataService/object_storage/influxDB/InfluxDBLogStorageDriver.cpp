/*
 * Copyright 2012, 18/06/2018 INFN
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

//#include <chaos/common/global.h>

#include "../../ChaosMetadataService.h"
#include <boost/algorithm/string.hpp>
#include <chaos_service_common/DriverPoolManager.h>

#include "InfluxDBLogStorageDriver.h"
#include "InfluxDB.h"
using namespace chaos;

using namespace chaos::service_common::persistence::data_access;

using namespace chaos::metadata_service;
using namespace chaos::service_common;

using namespace chaos::metadata_service::object_storage;
using namespace chaos::metadata_service::object_storage::abstraction;

#define INFO    INFO_LOG(InfluxDBLogStorageDriver)
#define DBG     DBG_LOG(InfluxDBLogStorageDriver)
#define ERR     ERR_LOG(InfluxDBLogStorageDriver)

DEFINE_CLASS_FACTORY(InfluxDBLogStorageDriver,
                     chaos::service_common::persistence::data_access::AbstractPersistenceDriver);

InfluxDBLogStorageDriver::InfluxDBLogStorageDriver(const std::string& name):
AbstractPersistenceDriver(name){}

InfluxDBLogStorageDriver::~InfluxDBLogStorageDriver() {}

void InfluxDBLogStorageDriver::init(void *init_data) throw (chaos::CException) {
    AbstractPersistenceDriver::init(init_data);

    const ChaosStringVector url_list = DriverPoolManager::logSetting.persistence_server_list;
    const std::string user = DriverPoolManager::logSetting.persistence_kv_param_map["user"];
    const std::string password = DriverPoolManager::logSetting.persistence_kv_param_map["pwd"];
    const std::string database = DriverPoolManager::logSetting.persistence_kv_param_map["db"];
    const std::string retention = DriverPoolManager::logSetting.persistence_kv_param_map["retention"];

    std::string servername="localhost";
    std::string funcpath="";
    std::string exptime="365d";
    int port=8086;
    if(url_list.size()>0){
        std::vector<std::string> ele;
        boost::regex expr{"(.+):(\\d+)/*(.*)"};
        boost::smatch what;
        if (boost::regex_search(url_list[0], what, expr)){
            
            servername=what[1];
            port=atoi(what[2].str().c_str());
            if(what.length()>=3){
                funcpath=what[3];
            }
        }
    /*    boost::split(ele,url_list[0],boost::is_any_of(":"));
        if(ele.size()>0){
            servername=ele[0];
        }
        if(ele.size()>1){
            port=atoi(ele[1].c_str());
        }*/
    }
    if(database.size()==0){
        ERR<<"You must specify a valid database name";
        throw chaos::CException(-1,"You must specify a valid database name",__FUNCTION__);
    }
    if(retention.size()){
        exptime=retention;
    }
    //influxdb_t  asyncdb = influxdb_t( new influxdb::async_api::simple_db(url_list[0], database));
   // asyncdb->with_authentication(user,password);
    DBG<<"server:"<<servername<<"\nport:"<<port<<"\ndatabase:"<<database<<"\nuser:"<<user<<"\npassw:"<<password<<" retention:"<<exptime<<" path:"<<funcpath;
    influxdb_cpp::server_info si(servername,port,database,user,password,"ms",exptime,funcpath);
    //register the data access implementations
    std::string resp;
    int ret;
    if((ret=influxdb_cpp::show_db(resp,si))<0){
       ERR<<"cannot show DB:"<<database<< " on:"<<servername<<" port:"<<port;
        throw chaos::CException(ret,"cannot connect or create DB:"+database+" on server:"+servername,__FUNCTION__);  
    }
    CDataWrapper r;
    r.setSerializedJsonData(resp.c_str());
    DBG<<" DB returned:"<<ret<<" answer:\""<<resp<<"\"";

    if((ret=influxdb_cpp::create_db(resp,database,si))<0){
       ERR<<"cannot connect or create DB:"<<database<< " on:"<<servername<<" port:"<<port;
    throw chaos::CException(ret,"cannot connect or create DB:"+database+" on server:"+servername,__FUNCTION__);  
    }
    r.setSerializedJsonData(resp.c_str());
    if(ret!=0){
        ERR<<" DB returned:"<<ret<<" answer:\""<<resp<<"\"";
        throw chaos::CException(ret,"Influx on server:"+servername+" error'"+resp+"'",__FUNCTION__);  

    } else{
            DBG<<" DB returned:"<<ret<<" answer:\""<<resp<<"\"";

    }
    registerDataAccess<ObjectStorageDataAccess>(new InfluxDB(si));
}

void InfluxDBLogStorageDriver::deinit() throw (chaos::CException) {
    //call sublcass
    AbstractPersistenceDriver::deinit();
}

void InfluxDBLogStorageDriver::deleteDataAccess(void *instance) {
    AbstractDataAccess *da_instance = static_cast<AbstractDataAccess*>(instance);
    if(da_instance != NULL)delete(da_instance);
}
