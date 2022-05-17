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
#include <chaos/common/io/QueryCursorRPC.h>
#include <chaos/common/network/NetworkBroker.h>
#include <chaos/common/message/MDSMessageChannel.h>
using namespace chaos::common::io;
using namespace chaos::common::data;
using namespace chaos::common::network;
using namespace chaos::common::direct_io::channel;
using namespace chaos::common::direct_io::channel::opcode_headers;

#define INFO    INFO_LOG(QueryCursorRPC)
#define DBG     DBG_LOG(QueryCursorRPC)
#define ERR     ERR_LOG(QueryCursorRPC)


static chaos::common::network::URLServiceFeeder buttami("",NULL); 
QueryCursorRPC::QueryCursorRPC(const std::string& _query_id,
                         const std::string& _node_id,
                         uint64_t _start_ts,
                         uint64_t _end_ts,
                         uint32_t default_page_len):QueryCursor(_query_id,buttami,_node_id,_start_ts,_end_ts,default_page_len)
{}

QueryCursorRPC::QueryCursorRPC(const std::string& _query_id,
                         const std::string& _node_id,
                         uint64_t _start_ts,
                         uint64_t _end_ts,
                         const ChaosStringSet& _meta_tags,
                         const ChaosStringSet& _projection_keys,
                         uint32_t default_page_len):QueryCursor(_query_id,buttami,_node_id,_start_ts,_end_ts,_meta_tags,_projection_keys,default_page_len)
{}

QueryCursorRPC::QueryCursorRPC(const std::string& _query_id,
                         const std::string& _node_id,
                         uint64_t _start_ts,
                         uint64_t _end_ts,
                         uint64_t _sequid,
                         uint64_t _runid,
                         uint32_t default_page_len):QueryCursor(_query_id,buttami,_node_id,_start_ts,_end_ts,_sequid,_runid,default_page_len)
{}

QueryCursorRPC::QueryCursorRPC(const std::string& _query_id,
                         const std::string& _node_id,
                         uint64_t _start_ts,
                         uint64_t _end_ts,
                         uint64_t _sequid,
                         uint64_t _runid,
                         const ChaosStringSet& _meta_tags,
                         const ChaosStringSet& _projection_keys,
                         uint32_t default_page_len):QueryCursor(_query_id,buttami,_node_id,_start_ts,_end_ts,_sequid,_runid,_meta_tags,_projection_keys,default_page_len)
{}


QueryCursorRPC::~QueryCursorRPC() {}


int QueryCursorRPC::fetchData() {
    try{
     api_error=chaos::common::network::NetworkBroker::getInstance()->getMetadataserverMessageChannel()->queryDataCloud(node_id,
                                                                       meta_tags,
                                                                       projection_keys,
                                                                       start_ts,
                                                                       end_ts,
                                                                       page_len,
                                                                       result_page.last_record_found_seq,
                                                                       result_page.found_element_page);


    if(api_error) {
        ERR << CHAOS_FORMAT("Error during fetching query page with code %1%", %api_error);
        phase = QueryPhaseEnded;
        return api_error;
    } else {
        result_page.current_fetched = 0;
        last_end_ts=result_page.last_record_found_seq.ts;
        DBG<<"retrieved:"<<result_page.found_element_page.size() <<" Page:"<<page_len<< " last ts:"<<last_end_ts << " ("<<chaos::common::utility::TimingUtil::toString(last_end_ts)<<")";

        if(result_page.found_element_page.size() < page_len) {
            phase = QueryPhaseEnded;
        }
    }
    } catch (const chaos::CException& e) {
    ERR << "Chaos Exception :" <<e.errorMessage;
  }catch (const std::exception& e) {
    ERR << " StdException :" <<e.what();
  }

    return api_error;
}
