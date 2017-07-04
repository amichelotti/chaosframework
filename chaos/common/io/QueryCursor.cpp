/*
 *	QueryCursor.cpp
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 06/09/16 INFN, National Institute of Nuclear Physics
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
    std::memset(&last_record_found_seq, 0, sizeof(direct_io::channel::opcode_headers::SearchSequence));
}

QueryCursor::ResultPage::~ResultPage() {}



const bool QueryCursor::ResultPage::hasNext() const {
    return current_fetched < found_element_page.size();
}

ChaosSharedPtr<chaos::common::data::CDataWrapper> QueryCursor::ResultPage::next() throw (chaos::CException){
    if(hasNext() == false) {throw CException(-1, "Cursor endend", __PRETTY_FUNCTION__);}
    return found_element_page[current_fetched++];
}

#pragma mark QueryCursor
QueryCursor::QueryCursor(const std::string& _query_id,
                         URLServiceFeeder& _connection_feeder,
                         const std::string& _node_id,
                         uint64_t _start_ts,
                         uint64_t _end_ts,
                         uint32_t default_page_len):
query_id(_query_id),
connection_feeder(_connection_feeder),
node_id(_node_id),
start_ts(_start_ts),
end_ts(_end_ts),
page_len(default_page_len),
phase(QueryPhaseNotStarted),
api_error(0){}

QueryCursor::~QueryCursor() {}

const std::string& QueryCursor::queryID() const {
    return query_id;
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
}

ChaosSharedPtr<chaos::common::data::CDataWrapper>  QueryCursor::next() throw (chaos::CException) {
    return result_page.next();
}

#pragma mark private methods
int64_t QueryCursor::fetchNewPage() {
    result_page.found_element_page.clear();
    IODirectIODriverClientChannels	*next_client = NULL;
    if((next_client = static_cast<IODirectIODriverClientChannels*>(connection_feeder.getService())) == NULL) return -1;
    
    //fetch the new page
    switch(phase) {
        case QueryPhaseNotStarted:
            DBG << "Start Search";
            std::memset(&result_page.last_record_found_seq, 0, sizeof(direct_io::channel::opcode_headers::SearchSequence));
            //change to the next phase
            phase = QueryPhaseStarted;
            break;
        case QueryPhaseStarted:
            DBG << "Continue on next page";
            break;
            
        case QueryPhaseEnded:
            ERR << "Cursor ended";
            return 0;
    }
    if((api_error = next_client->device_client_channel->queryDataCloud(node_id,
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

const uint32_t QueryCursor::getPageLen() const {
    return page_len;
}

void QueryCursor::setPageDimension(uint32_t new_page_len) {
    page_len = new_page_len;
}
