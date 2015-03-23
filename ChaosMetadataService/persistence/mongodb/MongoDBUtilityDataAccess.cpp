/*
 *	MongoDBUtilityDataAccess.cpp
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2015 INFN, National Institute of Nuclear Physics
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
#include "mongo_db_constants.h"
#include "MongoDBUtilityDataAccess.h"

#include <chaos/common/utility/TimingUtil.h>

#include <mongo/client/dbclient.h>

#define MDBUDA_INFO INFO_LOG(MongoDBUtilityDataAccess)
#define MDBUDA_DBG  DBG_LOG(MongoDBUtilityDataAccess)
#define MDBUDA_ERR  ERR_LOG(MongoDBUtilityDataAccess)

using namespace chaos::metadata_service::persistence::mongodb;
MongoDBUtilityDataAccess::MongoDBUtilityDataAccess(const boost::shared_ptr<service_common::persistence::mongodb::MongoDBHAConnectionManager>& _connection):
MongoDBAccessor(_connection){
    
}

MongoDBUtilityDataAccess::~MongoDBUtilityDataAccess() {
    
}


int MongoDBUtilityDataAccess::getNextSequenceValue(const std::string& sequence_name, uint64_t& next_value) {
    int err = 0;
    mongo::BSONObj          result;
    mongo::BSONObjBuilder   query;
    mongo::BSONObjBuilder   update;
    try {
        query << "seq" << sequence_name;
        mongo::BSONObj q = query.obj();
        

        update << "$inc" << BSON("value" << 1);
        mongo::BSONObj u = update.obj();
        
        DEBUG_CODE(MDBUDA_DBG << "getNextSequenceValue findAndModify ---------------------------------------------";)
        DEBUG_CODE(MDBUDA_DBG << "Query: "  << q.jsonString();)
        DEBUG_CODE(MDBUDA_DBG << "Update: "  << u.jsonString();)
        DEBUG_CODE(MDBUDA_DBG << "getNextSequenceValue findAndModify ---------------------------------------------";)
        if((err = connection->findAndModify(result, MONGO_DB_COLLECTION_NAME(getDatabaseName().c_str(), MONGODB_COLLECTION_NODES), q, u, true, true))) {
            MDBUDA_ERR << "Error updating ";
        }
    } catch (const mongo::DBException &e) {
        MDBUDA_ERR << e.what();
        err = -1;
    }
    return err;
}