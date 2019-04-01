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

#include <chaos/common/chaos_constants.h>
#include <chaos/common/io/QueryCursor.h>
#include <chaos/common/io/IODirectIODriver.h>

using namespace chaos::common::io;
using namespace chaos::common::data;
using namespace chaos::common::network;
using namespace chaos::common::direct_io::channel;
using namespace chaos::common::direct_io::channel::opcode_headers;

#define INFO    INFO_LOG(QueryCursor)
#define DBG     DBG_LOG(QueryCursor)
#define ERR     ERR_LOG(QueryCursor)

#pragma mark QueryCursor
QueryCursor::ResultPage::ResultPage():
current_fetched(0){
    //initlize last sequence number
    std::memset(&last_record_found_seq, 0, sizeof(direct_io::channel::opcode_headers::SearchSequence));
}

QueryCursor::ResultPage::~ResultPage() {}



const bool QueryCursor::ResultPage::hasNext() const {
    return current_fetched < found_element_page.size();
}
CUInt32 QueryCursor::ResultPage::size() const {
    return (CUInt32)found_element_page.size();
}
ChaosSharedPtr<chaos::common::data::CDataWrapper> QueryCursor::ResultPage::next() {
    if(hasNext() == false) {throw CException(-1, "Cursor endend", __PRETTY_FUNCTION__);}
    return found_element_page[current_fetched++];
}

#pragma mark QueryCursor
QueryCursor::QueryCursor(const std::string& _query_id,
                         URLServiceFeeder& _connection_feeder,
                         const std::string& _node_id,
                         CUInt64 _start_ts,
                         CUInt64 _end_ts,
                         CUInt32 default_page_len):
query_id(_query_id),
connection_feeder(_connection_feeder),
node_id(_node_id),
start_ts(_start_ts),
end_ts(_end_ts),
page_len(default_page_len),
phase(QueryPhaseNotStarted),
start_seq(0),
runid_seq(0),
meta_tags(ChaosStringSet()),
api_error(0){}

QueryCursor::QueryCursor(const std::string& _query_id,
                         URLServiceFeeder& _connection_feeder,
                         const std::string& _node_id,
                         CUInt64 _start_ts,
                         CUInt64 _end_ts,
                         const ChaosStringSet& _meta_tags,
                         CUInt32 default_page_len):
query_id(_query_id),
connection_feeder(_connection_feeder),
node_id(_node_id),
start_ts(_start_ts),
end_ts(_end_ts),
page_len(default_page_len),
phase(QueryPhaseNotStarted),
start_seq(0),
runid_seq(0),
meta_tags(_meta_tags),
api_error(0){}

QueryCursor::QueryCursor(const std::string& _query_id,
                         URLServiceFeeder& _connection_feeder,
                         const std::string& _node_id,
                         CUInt64 _start_ts,
                         CUInt64 _end_ts,
                         CUInt64 _sequid,
                         CUInt64 _runid,
                         CUInt32 default_page_len):
query_id(_query_id),
connection_feeder(_connection_feeder),
node_id(_node_id),
start_ts(_start_ts),
end_ts(_end_ts),
page_len(default_page_len),
phase(QueryPhaseNotStarted),
start_seq(_sequid),
runid_seq(_runid),
meta_tags(ChaosStringSet()),
api_error(0){
    if(_sequid>0){
        phase = QueryPhaseStarted;
        result_page.last_record_found_seq.run_id=_runid;
        result_page.last_record_found_seq.datapack_counter=_sequid-1;
    }
}

QueryCursor::QueryCursor(const std::string& _query_id,
                         URLServiceFeeder& _connection_feeder,
                         const std::string& _node_id,
                         CUInt64 _start_ts,
                         CUInt64 _end_ts,
                         CUInt64 _sequid,
                         CUInt64 _runid,
                         const ChaosStringSet& _meta_tags,
                         CUInt32 default_page_len):
query_id(_query_id),
connection_feeder(_connection_feeder),
node_id(_node_id),
start_ts(_start_ts),
end_ts(_end_ts),
page_len(default_page_len),
phase(QueryPhaseNotStarted),
start_seq(_sequid),
runid_seq(_runid),
meta_tags(_meta_tags),
api_error(0){
    if(_sequid>0){
        phase = QueryPhaseStarted;
        result_page.last_record_found_seq.run_id=_runid;
        result_page.last_record_found_seq.datapack_counter=_sequid-1;
    }
}

QueryCursor::~QueryCursor() {}

const std::string& QueryCursor::queryID() const {
    return query_id;
}
CUInt32  QueryCursor::size()const{
    return result_page.size();
}

const int32_t QueryCursor::getError(){return (int32_t)api_error;}
const bool QueryCursor::hasNext() {
    switch(phase) {
        case QueryPhaseNotStarted:
        case QueryPhaseStarted:
            if(result_page.hasNext() == false) {
                if(fetchNewPage()!=0){
                	ERR <<" Fetch returned error:"<<api_error;
                	return false;
                }
            }
            return result_page.hasNext();
            break;
        case QueryPhaseEnded:
            return result_page.hasNext();
    }
    return false;
}

ChaosSharedPtr<chaos::common::data::CDataWrapper>  QueryCursor::next()  {
    return result_page.next();
}

#pragma mark private methods
int QueryCursor::fetchNewPage() {
    result_page.found_element_page.clear();
    //fetch the new page
    switch(phase) {
        case QueryPhaseNotStarted:
//            std::memset(&result_page.last_record_found_seq, 0, sizeof(direct_io::channel::opcode_headers::SearchSequence));
//            result_page.last_record_found_seq.datapack_counter = -1;
            DBG << "["<<node_id<<"] start search "<<start_ts<<"-"<<end_ts<<" page_len:"<<page_len<<" data pack counter:"<< result_page.last_record_found_seq.datapack_counter<<"run id:"<< result_page.last_record_found_seq.run_id ;

            //change to the next phase
            phase = QueryPhaseStarted;
            break;
        case QueryPhaseStarted:
            //increase data pack count of last recod found that will be used has next counter id to fetch
            result_page.last_record_found_seq.datapack_counter++;
            DBG << "["<<node_id<<"] continue search  "<<start_ts<<"-"<<end_ts<<" page_len:"<<page_len<<" data pack counter:"<< result_page.last_record_found_seq.datapack_counter<<"run id:"<< result_page.last_record_found_seq.run_id ;
            break;
            
        case QueryPhaseEnded:

            ERR << "["<<node_id<<"] end search "<<start_ts<<"-"<<end_ts<<" page_len:"<<page_len<<" data pack counter:"<< result_page.last_record_found_seq.datapack_counter<<"run id:"<< result_page.last_record_found_seq.run_id ;


            return 0;
    }
    return fetchData();
}

int QueryCursor::fetchData() {
    IODirectIODriverClientChannels *next_client = NULL;
    if((next_client = static_cast<IODirectIODriverClientChannels*>(connection_feeder.getService())) == NULL) return -1;
    if((api_error = next_client->device_client_channel->queryDataCloud(node_id,
                                                                       meta_tags,
                                                                       start_ts,
                                                                       end_ts,
                                                                       page_len,
                                                                       result_page.last_record_found_seq,
                                                                       result_page.found_element_page))) {
        ERR << CHAOS_FORMAT("Error during fetching query page with code %1%", %api_error);
        phase = QueryPhaseEnded;
        return api_error;
    } else {
        result_page.current_fetched = 0;
        if(result_page.found_element_page.size() < page_len) {
            phase = QueryPhaseEnded;
        }
    }
    return api_error;
}

void QueryCursor::getIndexes(CUInt64& runid,CUInt64& seqid){
    runid = result_page.last_record_found_seq.run_id;
    seqid =result_page.last_record_found_seq.datapack_counter;
}

const CUInt32 QueryCursor::getPageLen() const {
    return page_len;
}

void QueryCursor::setPageDimension(CUInt32 new_page_len) {
    page_len = new_page_len;
}
