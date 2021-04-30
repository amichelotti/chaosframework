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
#ifndef __CHAOSFramework__NodeSearch__
#define __CHAOSFramework__NodeSearch__

#include "../AbstractApi.h"

namespace chaos {
namespace metadata_service {
namespace api {
namespace node {
class NodeSearch : public AbstractApi {
 protected:
 public:
  NodeSearch();
  ~NodeSearch();
  chaos::common::data::CDWUniquePtr execute(chaos::common::data::CDWUniquePtr api_data);
  std::vector<std::string>          search(const std::string& name, const std::string& type ,const std::string& impl = "", uint32_t maxres = 100000);

  std::vector<std::string>          search(const std::string& name, const chaos::NodeType::NodeSearchType nt=chaos::NodeType::node_type_all,const std::string& impl = "", uint32_t maxres = 100000);
};

}  // namespace node
}  // namespace api
}  // namespace metadata_service
}  // namespace chaos

#endif /* defined(__CHAOSFramework__NodeSearch__) */
