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
#include "NodeSearch.h"
#include "../../ChaosMetadataService.h"

#define NS_INFO INFO_LOG(NodeSearch)
#define NS_DBG DBG_LOG(NodeSearch)
#define NS_ERR ERR_LOG(NodeSearch)

using namespace chaos::common::data;
using namespace chaos::metadata_service::api::node;
using namespace chaos::metadata_service::persistence::data_access;

CHAOS_MDS_DEFINE_API_CLASS_CD(NodeSearch, "nodeSearch");

CDWUniquePtr NodeSearch::execute(CDWUniquePtr api_data) {
  CHECK_CDW_THROW_AND_LOG(api_data, NS_ERR, -1, "No parameter found")
  CHECK_KEY_THROW_AND_LOG(api_data, "unique_id_filter", NS_ERR, -2, CHAOS_FORMAT("The %1% key is mandatory", % "unique_id_filter"));
  CHECK_KEY_THROW_AND_LOG(api_data, "node_type_filter", NS_ERR, -3, CHAOS_FORMAT("The %1% key is mandatory", % "node_type_filter"));

  bool                               alive_only       = false;
  uint32_t                           last_sequence_id = 0;
  uint32_t                           page_length      = 30;
  chaos::common::data::CDataWrapper *result           = NULL;

  if (api_data->hasKey("last_node_sequence_id") &&
      api_data->isInt32Value("last_node_sequence_id")) {
    last_sequence_id = (uint32_t)api_data->getInt32Value("last_node_sequence_id");
    //   NS_DBG << "Need to load a new page starting from id:" << last_sequence_id;
  }

  if (api_data->hasKey("result_page_length") &&
      api_data->isInt32Value("result_page_length")) {
    page_length = (uint32_t)api_data->getInt32Value("result_page_length");
    //     NS_DBG << "The length of the page has been set to:" << page_length;
  }
  if (api_data->hasKey("alive_only") &&
      api_data->isBoolValue("alive_only")) {
    alive_only = api_data->getBoolValue("alive_only");
    //  NS_DBG << "Search only item that are with alive state:" << alive_only <<" last seq:"<<last_sequence_id<<" page:"<<page_length;
  }
  //get node data access
  chaos::NodeType::NodeSearchType search_type   = (chaos::NodeType::NodeSearchType)api_data->getInt32Value("node_type_filter");
  std::string                     search_filter = api_data->getStringValue("unique_id_filter");
  std::string                     impl;
  if (api_data->hasKey("impl")) {
    impl = api_data->getStringValue("impl");
  }
  if (search_type == chaos::NodeType::node_type_variable) {
    GET_DATA_ACCESS(UtilityDataAccess, u_da, -4);
    if (u_da->searchVariable(&result, search_filter, last_sequence_id, page_length)) {
      LOG_AND_TROW(NS_ERR, -5, "Searching variable: " + search_filter)
    }
  } else {
          GET_DATA_ACCESS(NodeDataAccess, n_da, -4)

#if 1

    if (n_da->searchNode(&result,
                         search_filter,
                         search_type,
                         alive_only,
                         last_sequence_id,
                         page_length,
                         impl)) {
      LOG_AND_TROW(NS_ERR, -5, "Loading Searching: " + search_filter)
    } 
#else
    if (n_da->searchNode(&result,
                         search_filter,
                         search_type,
                         false,
                         last_sequence_id,
                         page_length,
                         impl)) {
      LOG_AND_TROW(NS_ERR, -5, "Loading Searching: " + search_filter)
    }
    if (!alive_only) {
      return CDWUniquePtr(result);
    }

    if (result&&result->hasKey(chaos::NodeType::NODE_SEARCH_LIST_KEY)&&result->isVectorValue(chaos::NodeType::NODE_SEARCH_LIST_KEY)) {
      chaos::common::data::CMultiTypeDataArrayWrapperSPtr v = result->getVectorValue(chaos::NodeType::NODE_SEARCH_LIST_KEY);
      for (int cnt = 0; v.get()&&(cnt < v->size()); cnt++) {
        bool         alive = false;
        CDWUniquePtr val   = v->getCDataWrapperElementAtIndex(cnt);
        if (val.get() && val->hasKey(chaos::NodeDefinitionKey::NODE_UNIQUE_ID)) {
          alive = ChaosMetadataService::getInstance()->isNodeAlive(val->getStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID));
        }
        if (!alive) {
          v->removeElementAtIndex(cnt);
        }
      }
    }
#endif
  }
  return CDWUniquePtr(result);
}
std::vector<std::string> NodeSearch::search(const std::string&unique_id_filter,const std::string& type,const std::string&impl,uint32_t maxres){
  return search(unique_id_filter,chaos::NodeType::human2NodeType(type),impl,maxres);
}
std::vector<std::string> NodeSearch::search(const std::string&unique_id_filter,const chaos::NodeType::NodeSearchType type,const std::string&impl,uint32_t maxres){
ChaosUniquePtr<chaos::common::data::CDataWrapper> message(new CDataWrapper());
std::vector<std::string> node_found;
    message->addStringValue("unique_id_filter", unique_id_filter);
    if (impl.size() > 0)
      message->addStringValue("impl", impl);

    message->addInt32Value("node_type_filter", (int32_t)type);
    message->addBoolValue("alive_only", false);
    message->addInt32Value("result_page_length", maxres);
    CDWUniquePtr res = execute(MOVE(message));

    if (res.get() &&
        res->hasKey(chaos::NodeType::NODE_SEARCH_LIST_KEY) &&
        res->isVectorValue(chaos::NodeType::NODE_SEARCH_LIST_KEY)) {
      //we have result
      CMultiTypeDataArrayWrapperSPtr snapshot_desc_list = res->getVectorValue(chaos::NodeType::NODE_SEARCH_LIST_KEY);
      for (int idx = 0;
           idx < snapshot_desc_list->size();
           idx++) {
        ChaosUniquePtr<chaos::common::data::CDataWrapper> element(snapshot_desc_list->getCDataWrapperElementAtIndex(idx));
        if (element.get() &&
            element->hasKey(chaos::NodeDefinitionKey::NODE_UNIQUE_ID)) {
          node_found.push_back(element->getStringValue(chaos::NodeDefinitionKey::NODE_UNIQUE_ID));
          
        }
      }
    }
    return node_found;
}
