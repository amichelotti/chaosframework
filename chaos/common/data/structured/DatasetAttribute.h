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

#ifndef __CHAOSFramework_DD3CE2CE_DFAD_42AA_8C8E_8AC82B03C5FD_DatasetAttribute_h
#define __CHAOSFramework_DD3CE2CE_DFAD_42AA_8C8E_8AC82B03C5FD_DatasetAttribute_h

#include <chaos/common/chaos_types.h>
#include <chaos/common/chaos_constants.h>
#include <chaos/common/data/TemplatedDataSDWrapper.h>

namespace chaos {
    namespace common {
        namespace data {
            namespace structured {
                CHAOS_DEFINE_VECTOR_FOR_TYPE(unsigned int, DatasetSubtypeList)
                
                //! The description of a n attribute of a CHAOS dataset
                struct DatasetAttribute {
                    std::string                                     name;
                    std::string                                     description;
                    std::string                                     min_value;
                    std::string                                     max_value;
                    std::string                                     default_value;
                    std::string                                     increment;
                    std::string                                     unit;
                    std::string                                     convf; //conversion factor
                    std::string                                     offset;//offset

                    std::string                                     warningThreshold;//offset
                    std::string                                     errorThreshold;//offset

                    uint32_t                                        max_size;
                    chaos::DataType::DataSetAttributeIOAttribute    direction;
                    chaos::DataType::DataType                       type;
                    DatasetSubtypeList                              binary_subtype_list;
                    uint32_t                                        binary_cardinality;
                    
                    DatasetAttribute();
                    DatasetAttribute(const std::string& attr_name,
                                     const std::string& attr_description,
                                     const chaos::DataType::DataType& attr_type);
                    DatasetAttribute(const std::string& attr_name,
                                     const std::string& attr_description,
                                     const chaos::DataType::DataType& attr_type,
                                     const chaos::DataType::DataSetAttributeIOAttribute io,
                                     const std::string& min,
                                     const std::string& max,
                                     const std::string& def,
                                     const std::string& inc,
                                     const std::string& unit);

                    DatasetAttribute(const DatasetAttribute& copy_src);
                    DatasetAttribute& operator=(DatasetAttribute const &rhs);
                };
                
                
                //!helper for create or read the script description
                CHAOS_OPEN_SDWRAPPER(DatasetAttribute)
                    void deserialize(chaos::common::data::CDataWrapper *serialized_data) {
                        if(serialized_data == NULL) return;
                        Subclass::dataWrapped().binary_subtype_list.clear();
                        Subclass::dataWrapped().name = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_NAME, "");
                        Subclass::dataWrapped().description = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DESCRIPTION, "");
                        Subclass::dataWrapped().direction = static_cast<chaos::DataType::DataSetAttributeIOAttribute>(CDW_GET_INT32_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DIRECTION, chaos::DataType::Bidirectional));
                        Subclass::dataWrapped().type = static_cast<chaos::DataType::DataType>(CDW_GET_INT32_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_TYPE, chaos::DataType::TYPE_UNDEFINED));
                        Subclass::dataWrapped().min_value = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_MIN_RANGE, "");
                        Subclass::dataWrapped().max_value = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_MAX_RANGE, "");
                        Subclass::dataWrapped().increment = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_INCREMENT, "0");
                        Subclass::dataWrapped().unit = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_UNIT, "NA");
                        Subclass::dataWrapped().convf = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_CONVFACT, "1");
                        Subclass::dataWrapped().offset = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_OFFSET, "0");
                        Subclass::dataWrapped().warningThreshold = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_WARN_THR, "0");
                        Subclass::dataWrapped().errorThreshold = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_ERROR_THR, "0");

                        Subclass::dataWrapped().default_value = CDW_GET_SRT_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DEFAULT_VALUE, "");
                        Subclass::dataWrapped().max_size = (uint32_t)CDW_GET_INT32_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_VALUE_MAX_SIZE, 0);
                        Subclass::dataWrapped().binary_cardinality = CDW_GET_INT32_WITH_DEFAULT(serialized_data, chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_BINARY_CARDINALITY, 0);
                        
                        if(Subclass::dataWrapped().type == DataType::TYPE_BYTEARRAY) {
                            if(serialized_data->hasKey(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_BINARY_SUBTYPE)) {
                                if(serialized_data->isVectorValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_BINARY_SUBTYPE)) {
                                    //multiple subtype
                                    CMultiTypeDataArrayWrapperSPtr serialized_array = serialized_data->getVectorValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_BINARY_SUBTYPE);
                                    for(int idx = 0;
                                        idx < serialized_array->size();
                                        idx++) {
                                        Subclass::dataWrapped().binary_subtype_list.push_back(static_cast<unsigned int>(serialized_array->getInt32ElementAtIndex(idx)));
                                    }
                                } else {
                                    //single subtype
                                    Subclass::dataWrapped().binary_subtype_list.push_back(static_cast<chaos::DataType::BinarySubtype>(serialized_data->getInt32Value(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_BINARY_SUBTYPE)));
                                }
                            }
                        }
                    }
                    
                    ChaosUniquePtr<chaos::common::data::CDataWrapper> serialize() {
                        ChaosUniquePtr<chaos::common::data::CDataWrapper> data_serialized(new chaos::common::data::CDataWrapper());
                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_NAME, Subclass::dataWrapped().name);
                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DESCRIPTION, Subclass::dataWrapped().description);
                        data_serialized->addInt32Value(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DIRECTION, Subclass::dataWrapped().direction);
                        data_serialized->addInt32Value(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_TYPE, Subclass::dataWrapped().type);
                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_MIN_RANGE, Subclass::dataWrapped().min_value);
                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_MAX_RANGE, Subclass::dataWrapped().max_value);
                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_INCREMENT, Subclass::dataWrapped().increment);
                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_UNIT, Subclass::dataWrapped().unit);

                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_CONVFACT, Subclass::dataWrapped().convf);
                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_OFFSET, Subclass::dataWrapped().offset);

                        data_serialized->addStringValue(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_DEFAULT_VALUE, Subclass::dataWrapped().default_value);
                        data_serialized->addInt32Value(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_VALUE_MAX_SIZE, Subclass::dataWrapped().max_size);
                        data_serialized->addInt32Value(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_BINARY_CARDINALITY, Subclass::dataWrapped().binary_cardinality);
                        if((Subclass::dataWrapped().type == DataType::TYPE_BYTEARRAY) &&
                           Subclass::dataWrapped().binary_subtype_list.size()) {
                            if(Subclass::dataWrapped().binary_subtype_list.size() > 1) {
                                //multiple value
                                for(DatasetSubtypeListIterator it = Subclass::dataWrapped().binary_subtype_list.begin(),
                                    end = Subclass::dataWrapped().binary_subtype_list.end();
                                    it != end;
                                    it++) {
                                    data_serialized->appendInt32ToArray(*it);
                                }
                                data_serialized->finalizeArrayForKey(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_BINARY_SUBTYPE);
                            } else {
                            
                                data_serialized->addInt32Value(chaos::ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_BINARY_SUBTYPE, Subclass::dataWrapped().binary_subtype_list[0]);
                            }
                        }
                        return data_serialized;
                    }
                CHAOS_CLOSE_SDWRAPPER()
                
                //!a list of a script base information usefullt for search operation
//                template<typename W = chaos::common::data::CopySDWrapper<DatasetAttribute> >
//                chaos::common::data::TemplatedDataListWrapper<DatasetAttribute, DatasetAttributeSDWrapper<W> > DatasetAttributeListWrapper;
                
                
                //Definition of dataset attribute list
                CHAOS_DEFINE_VECTOR_FOR_TYPE(chaos::common::data::structured::DatasetAttribute,
                                             DatasetAttributeList);
            }
            

        }
    }
}
chaos::common::data::CDataWrapper& operator<<(chaos::common::data::CDataWrapper& dst,const chaos::common::data::structured::DatasetAttribute&src);
chaos::common::data::structured::DatasetAttribute&  operator>>(chaos::common::data::structured::DatasetAttribute& dst,const chaos::common::data::CDataWrapper& src );

#endif /* __CHAOSFramework_DD3CE2CE_DFAD_42AA_8C8E_8AC82B03C5FD_DatasetAttribute_h */
