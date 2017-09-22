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
#ifndef __CHAOSFramework__ProducerRegisterAPI__
#define __CHAOSFramework__ProducerRegisterAPI__

#include "../AbstractApi.h"

#include <boost/shared_ptr.hpp>
namespace chaos {
	namespace wan_proxy {
		namespace api {
			namespace producer {
				
				typedef enum ProducerRegisterDatasetApiErrorCode {
					
				}ProducerRegisterDatasetApiErrorCode;
				
				class ProducerRegisterDatasetApi:
				public AbstractApi {
					int scanDatasetElement(const Json::Value& dataset_json_element,
										   std::string& err_msg,
										   ChaosSharedPtr<chaos::common::data::CDataWrapper>& element);
				public:
					//! default constructor
					ProducerRegisterDatasetApi(persistence::AbstractPersistenceDriver *_persistence_driver);
					
					//! default destructor
					~ProducerRegisterDatasetApi();
					
					//! execute the api
					/*!
					 The name of the producer is given in the uri. The json  need to be composed as follow:
					 {
					 - the timestamp of the producer got at the registration time
					 "ds_timestamp": number,
					 
					 - the dataset of the producer that is an array of json document
					 "ds_desc": [ {
					 -the name of the attribute
					 "ds_attr_name": string,
					 
					 -the description fo the attribute
					 "ds_attr_desc": string,
					 
					 -the type of the attribute as: int32, int64, double, string, binary, boolean
					 "ds_attr_type": string
					 
					 - the direction of the attribute o for "input" attribute "output" otherwise.
					 
					 - output are the attribute the are emitted by producer
					 "ds_attr_dir": string,
					 
					 - the maximum value of the attribute (when applicable)[optional only for input attribute]
					 "ds_max_range": string,
					 
					 - the minimum value of the attribute (when applicable)[optional only for input attribute]
					 "ds_min_range": string,
					 
					 - the default value of the attribute (when applicable)[optional only for input attribute]
					 "ds_default_value": "1"
					 }]
					 }
					 
					 */
					int execute(std::vector<std::string>& api_tokens,
								const Json::Value& input_data,
								std::map<std::string, std::string>& output_header,
								Json::Value& output_data);
				};
				
			}
		}
	}
}

#endif /* defined(__CHAOSFramework__ProducerRegisterDatasetApi__) */
