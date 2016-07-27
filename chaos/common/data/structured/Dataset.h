/*
 *	Dataset.h
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 25/07/16 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFramework__9AF21F8_5C6D_4BA4_B954_150112079D8D_Dataset_h
#define __CHAOSFramework__9AF21F8_5C6D_4BA4_B954_150112079D8D_Dataset_h

#include <chaos/common/chaos_constants.h>

#include <chaos/common/chaos_types.h>
#include <chaos/common/data/TemplatedDataSDWrapper.h>
#include <chaos/common/data/structured/DatasetAttribute.h>

#include <boost/shared_ptr.hpp>

#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace chaos {
    namespace common {
        namespace data {
            namespace structured {
            
                typedef boost::shared_ptr<DatasetAttribute> DatasetAttributePtr;
                
                //! define the contaner for the dataset within the boost multi index set
                struct DatasetAttributeElement {
                    
                    typedef boost::shared_ptr<DatasetAttributeElement> DatasetAttributeElementPtr;
                    
                    //!keep track of ordering id
                    unsigned int seq_id;
                    
                    //dataset pointer
                    DatasetAttributePtr dataset_attribute;
                    
                    DatasetAttributeElement(unsigned int _seq_id,
                                            DatasetAttributePtr _dataset);
                    
                    struct less {
                        bool operator()(const DatasetAttributeElementPtr& h1, const DatasetAttributeElementPtr& h2);
                    };
                    
                    struct extract_key {
                        typedef std::string result_type;
                        // modify_key() requires return type to be non-const
                        const result_type &operator()(const DatasetAttributeElementPtr &p) const;
                    };
                    
                    struct extract_type {
                        typedef chaos::DataType::DataType result_type;
                        // modify_key() requires return type to be non-const
                        const result_type &operator()(const DatasetAttributeElementPtr &p) const;
                    };
                    
                    struct extract_ordered_id {
                        typedef unsigned int result_type;
                        // modify_key() requires return type to be non-const
                        const result_type &operator()(const DatasetAttributeElementPtr &p) const;
                    };
                };
                
                //tag
                struct DAETagName{};
                struct DAETagOrderedId{};
                
                //multi-index set
                typedef boost::multi_index_container<
                DatasetAttributeElement::DatasetAttributeElementPtr,
                boost::multi_index::indexed_by<
                boost::multi_index::ordered_unique<boost::multi_index::tag<DAETagOrderedId>,  DatasetAttributeElement::extract_ordered_id>,
                boost::multi_index::hashed_unique<boost::multi_index::tag<DAETagName>,  DatasetAttributeElement::extract_key>
                >
                > DatasetAttributeElementContainer;
                
                //!priority index and iterator
                typedef boost::multi_index::index<DatasetAttributeElementContainer, DAETagOrderedId>::type                      DECOrderedIndex;
                typedef boost::multi_index::index<DatasetAttributeElementContainer, DAETagOrderedId>::type::iterator            DECOrderedIndexIterator;
                typedef boost::multi_index::index<DatasetAttributeElementContainer, DAETagOrderedId>::type::reverse_iterator    DECOrderedIndexReverseIterator;
                
                //!name index and iterator
                typedef boost::multi_index::index<DatasetAttributeElementContainer, DAETagName>::type                           DECNameIndex;
                typedef boost::multi_index::index<DatasetAttributeElementContainer, DAETagName>::type::iterator                 DECNameIndexIterator;

                CHAOS_DEFINE_VECTOR_FOR_TYPE(DatasetAttributePtr ,DatasetPtrVector);
                
                //! The description of a complete dataset with his attribute and property
                struct Dataset {
                    //is the name of the dataset
                    std::string                         name;
                    //is the type of the dataset
                    chaos::DataType::DatasetType        type;
                    //is the key that need to be used to searh dataset on the shared memory
                    std::string                         dataset_key;
                    //is the ocmplete list of the attribute of the dataset
                    DatasetAttributeElementContainer    attribute_set;
                    
                    Dataset();
                    Dataset(const std::string& _name,
                            const chaos::DataType::DatasetType& _type);
                    Dataset(const Dataset& copy_src);
                    Dataset& operator=(Dataset const &rhs);
                    
                    int addAttribute(DatasetAttributePtr new_attribute);
                    
                    DatasetAttributePtr getAttributebyName(const std::string& attr_name);
                    
                    DatasetAttributePtr getAttributebyOrderedIDe(const unsigned int ordered_id);
                };
                
                //! define serialization wrapper for dataset type
                CHAOS_DEFINE_TEMPLATED_DATA_SDWRAPPER_CLASS(Dataset) {
                public:
                    CHAOS_DECLARE_SD_WRAPPER_CONSTRUCTOR(Dataset)
                    
                    void deserialize(chaos::common::data::CDataWrapper *serialized_data);
                    
                    std::auto_ptr<chaos::common::data::CDataWrapper> serialize();
                };
            }
        }
    }
}

#endif /* __CHAOSFramework__9AF21F8_5C6D_4BA4_B954_150112079D8D_Dataset_h */
