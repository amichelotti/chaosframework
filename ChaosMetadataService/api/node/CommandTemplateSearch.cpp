/*
 *	CommandTemplateSearch.cpp
 *	!CHAOS
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

#include "CommandTemplateSearch.h"

#include <boost/format.hpp>

using namespace chaos;
using namespace chaos::common::data;
using namespace chaos::common::batch_command;
using namespace chaos::metadata_service::api::node;
using namespace chaos::metadata_service::persistence::data_access;

#define N_TCS_INFO INFO_LOG(CommandTemplateSearch)
#define N_TCS_DBG  DBG_LOG(CommandTemplateSearch)
#define N_TCS_ERR  ERR_LOG(CommandTemplateSearch)

CommandTemplateSearch::CommandTemplateSearch():
AbstractApi("commandTemplateSearch"){
    
}

CommandTemplateSearch::~CommandTemplateSearch() {
    
}

CDataWrapper *CommandTemplateSearch::execute(CDataWrapper *api_data,
                                             bool& detach_data) throw(chaos::CException) {
    int err = 0;
    CDataWrapper *result = NULL;
    uint32_t last_sequence_id = 0;
    uint32_t page_length = 100;
    std::vector<std::string> cmd_uid_filter;
    
    CHECK_CDW_THROW_AND_LOG(api_data, N_TCS_ERR, -1, "No parameter found")
    CHECK_KEY_THROW_AND_LOG(api_data, "cmd_uid_fetch_list", N_TCS_ERR, -2, "The attribute cmd_uid_fetch_list is mandatory")
    
    GET_DATA_ACCESS(NodeDataAccess, n_da, -3)
    
    if(api_data->hasKey("last_node_sequence_id")) {
        last_sequence_id = (uint32_t)api_data->getInt32Value("last_node_sequence_id");
        N_TCS_DBG << "Need to load a new page starting from id:" << last_sequence_id;
    }
    if(api_data->hasKey("result_page_length")) {
        page_length = (uint32_t)api_data->getInt32Value("result_page_length");
        N_TCS_DBG << "The length of the page has been set to:" << page_length;
    }
    
    std::auto_ptr<CMultiTypeDataArrayWrapper> uid_list(api_data->getVectorValue("cmd_uid_fetch_list"));
    //we don't accept request for query on all command
    if(uid_list->size()==0) return result;
    
    //fill the vector
    for(int idx = 0;
        idx < uid_list->size();
        idx++) {
        cmd_uid_filter.push_back(uid_list->getStringElementAtIndex(idx));
    }
    
    //start the query
    if((err = n_da->searchCommandTemplate(&result,
                                          cmd_uid_filter,
                                          last_sequence_id,
                                          page_length))) {
        LOG_AND_TROW(N_TCS_ERR, -4, "Loading search page")
    }
    return result;
}
