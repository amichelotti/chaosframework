/*
 *	DataBrokerEditor.cpp
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

#include <chaos/cu_toolkit/data_manager/manipulation/DataBrokerEditor.h>

using namespace chaos::DataType;
using namespace chaos::common::data;
using namespace chaos::common::data::structured;
using namespace chaos::cu::data_manager::manipulation;

#define INFO INFO_LOG(DataBrokerEditor)
#define DBG DBG_LOG(DataBrokerEditor)
#define ERR ERR_LOG(DataBrokerEditor)

DataBrokerEditor::DataBrokerEditor():
ds_index_name(container_dataset.get<DatasetElementTagName>()),
ds_index_ordered(container_dataset.get<DatasetElementTagOrderedId>()),
ds_index_type_name(container_dataset.get<DatasetElementTagTypeName>()){}

DataBrokerEditor::~DataBrokerEditor() {}

int DataBrokerEditor::addNewDataset(const std::string& name,
                                    const DatasetType type,
                                    const std::string& shared_key) {
    DECNameIndexIterator nit = ds_index_name.find(name);
    if(nit != ds_index_name.end()) {
        //a dataset with name already exists
        ERR << CHAOS_FORMAT("A dataset with name %1% already exists", %name);
        return -1;
    }
    
    //add new dataset
    container_dataset.insert(DatasetElement::DatasetElementPtr(new DatasetElement((unsigned int)container_dataset.size(),
                                                                                  DatasetPtr(new Dataset(name, type)))));
    return 0;
}

int DataBrokerEditor::addAttributeToDataset(const std::string& ds_name,
                                            const std::string& attr_name,
                                            const std::string& attr_description,
                                            const chaos::DataType::DataType attr_type,
                                            uint32_t maxSize) {
    int err = 0;
    DECNameIndexIterator nit = ds_index_name.find(ds_name);
    if(nit != ds_index_name.end()) {
        //a dataset with name already exists
        ERR << CHAOS_FORMAT("No dataset with name %1% found", %ds_name);
        err = -1;
    } else {
        //we can manipualte dataset adding new attrbiute
        DatasetAttributePtr new_attribute(new DatasetAttribute(attr_name,
                                                               attr_description,
                                                               attr_type));
        //add new attribute
        if((err = (*nit)->dataset->addAttribute(new_attribute))) {
            ERR << CHAOS_FORMAT("Error adding new attribute %1% into the dataset %2%", %attr_name%ds_name);
        } else {
            //add new cache for attribute
            (*nit)->dataset_value_cache.addAttribute(attr_name,
                                                     maxSize,
                                                     attr_type);
        }
    }
    return err;
}

int DataBrokerEditor::addBinaryAttributeAsSubtypeToDataSet(const std::string&          ds_name,
                                                           const std::string&          attr_name,
                                                           const std::string&          attr_description,
                                                           DataType::BinarySubtype     attr_subtype,
                                                           int32_t                     attr_cardinality) {
    //add the attribute
    int err = addAttributeToDataset(ds_name,
                                    attr_name,
                                    attr_description,
                                    DataType::TYPE_BYTEARRAY);
    if(err) {return err;}
    
    //set other value of the attribute
    DECNameIndexIterator nit = ds_index_name.find(ds_name);
    DatasetAttributePtr attribute_ptr = (*nit)->dataset->getAttributebyName(attr_name);
    attribute_ptr->binary_subtype_list.push_back(attr_subtype);
    attribute_ptr->binary_cardinality = attr_cardinality;
    return err;
}

int DataBrokerEditor::addBinaryAttributeAsSubtypeToDataSet(const std::string&           ds_name,
                                                           const std::string&           attr_name,
                                                           const std::string&           attr_description,
                                                           const DatasetSubtypeList&    attr_subtype_list,
                                                           int32_t                      attr_cardinality) {
    //add the attribute
    int err = addAttributeToDataset(ds_name,
                                    attr_name,
                                    attr_description,
                                    DataType::TYPE_BYTEARRAY);
    if(err) {return err;}
    
    //set other value of the attribute
    DECNameIndexIterator nit = ds_index_name.find(ds_name);
    DatasetAttributePtr attribute_ptr = (*nit)->dataset->getAttributebyName(attr_name);
    attribute_ptr->binary_subtype_list = attr_subtype_list;
    attribute_ptr->binary_cardinality = attr_cardinality;
    return err;
}

int DataBrokerEditor::addBinaryAttributeAsMIMETypeToDataSet(const std::string& ds_name,
                                                            const std::string& attr_name,
                                                            const std::string& attr_description,
                                                            const std::string& attr_mime_type) {
    //add the attribute
    int err = addAttributeToDataset(ds_name,
                                    attr_name,
                                    attr_description,
                                    DataType::TYPE_BYTEARRAY);
    if(err) {return err;}
    
    //set other value of the attribute
    DECNameIndexIterator nit = ds_index_name.find(ds_name);
    DatasetAttributePtr attribute_ptr = (*nit)->dataset->getAttributebyName(attr_name);
    return err;
}

int DataBrokerEditor::setOutputAttributeValue(const std::string&  ds_name,
                                              const std::string&  attr_name,
                                              void * value,
                                              uint32_t size) {
    DECNameIndexIterator nit = ds_index_name.find(ds_name);
    if(nit == ds_index_name.end()) return -1;
    //we have the dataset
    chaos::common::data::cache::AttributeValue *value_setting = (*nit)->dataset_value_cache.getValueSettingByName(attr_name);
    if(value_setting) {
        value_setting->setValue(value,
                                size);
    }
    return 0;
}

int DataBrokerEditor::setOutputAttributeValue(const std::string& ds_name,
                                              const unsigned int attr_index,
                                              void * value,
                                              uint32_t size) {
    DECNameIndexIterator nit = ds_index_name.find(ds_name);
    if(nit == ds_index_name.end()) return -1;
    //we have the dataset
    chaos::common::data::cache::AttributeValue *value_setting = (*nit)->dataset_value_cache.getValueSettingForIndex(attr_index);
    if(value_setting) {
        value_setting->setValue(value,
                                size);
    }
    return 0;
}

std::auto_ptr<CDataWrapper> DataBrokerEditor::serialize() {
    std::auto_ptr<CDataWrapper> result(new CDataWrapper());
    //scan all dataset and every serialization will be added to global CDataWrapper as array
    DatasetSDWrapper< ReferenceSDWrapper<Dataset> > reference_ser_wrap;
    for(DECOrderedIndexIterator oit = ds_index_ordered.begin(),
        oend = ds_index_ordered.end();
        oit != oend;
        oit++ ) {
        //set the reference
        reference_ser_wrap() = *(*oit)->dataset;
        //append dataset serialization
        result->appendCDataWrapperToArray(*reference_ser_wrap.serialize());
    }
    result->finalizeArrayForKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DESCRIPTION);
    return result;
}

void DataBrokerEditor::deserialize(CDataWrapper& serialization) {
    if( !serialization.hasKey(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DESCRIPTION) ||
       serialization.isVectorValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DESCRIPTION)) return;
    
    //get the datawrapper array
    std::auto_ptr<CMultiTypeDataArrayWrapper> ser_ds_vec(serialization.getVectorValue(ControlUnitNodeDefinitionKey::CONTROL_UNIT_DATASET_ATTRIBUTE_DESCRIPTION));
    if(ser_ds_vec->size() == 0) return;
    
    DatasetSDWrapper<> ds_wrapper;
    for(int idx =0;
        idx < ser_ds_vec->size();
        idx++) {
        std::auto_ptr<CDataWrapper> ds_ser(ser_ds_vec->getCDataWrapperElementAtIndex(idx));
        ds_wrapper.deserialize(ds_ser.get());
        addNewDataset(ds_wrapper.dataWrapped());
    }
}

#pragma mark Private Methods
int DataBrokerEditor::addNewDataset(Dataset& new_dataset) {
    int err = 0;
    DECTypeNameIndexIterator dit = ds_index_type_name.find(boost::make_tuple(new_dataset.type,
                                                                             new_dataset.name));
    if(dit == ds_index_type_name.end()) {
        // dataset already exists
        container_dataset.insert(DatasetElement::DatasetElementPtr(new DatasetElement((unsigned int)container_dataset.size(),
                                                                                      DatasetPtr(new Dataset(new_dataset)))));
    } else {
        //scan all new dataset attribute if there are new attribute they will be added
        //! if attribute is found, property are updated with new daset one.
        for(unsigned int idx = 0;
            idx < new_dataset.getAttributeSize();
            idx++){
            DatasetAttributePtr attr_new_ds = new_dataset.getAttributebyOrderedID(idx);
            if((*dit)->dataset->hasAttribute(attr_new_ds->name)) {
                //get and update
                DatasetAttributePtr attr_local_ds = (*dit)->dataset->getAttributebyName(attr_new_ds->name);
                CHAOS_ASSERT(attr_local_ds.get());
                //update attribute
                updateAttributePorertyForDataset(*attr_new_ds,
                                                 *attr_local_ds);
            } else {
                //add new attribute
                //add new attribute
                if((err = (*dit)->dataset->addAttribute(attr_new_ds))) {
                    ERR << CHAOS_FORMAT("Error adding new attribute %1% into the dataset %2%", %attr_new_ds->name%new_dataset.name);
                } else {
                    //add new cache for attribute
                    (*dit)->dataset_value_cache.addAttribute(attr_new_ds->name,
                                                             attr_new_ds->max_size,
                                                             attr_new_ds->type);
                }
            }
        }
    }
    return 0;
}

void DataBrokerEditor::updateAttributePorertyForDataset(DatasetAttribute attr_src,
                                                        DatasetAttribute attr_dst) {
    
    attr_dst.min_value = attr_src.min_value;
    attr_dst.max_value = attr_src.max_value;
    attr_dst.default_value = attr_src.default_value;
}
