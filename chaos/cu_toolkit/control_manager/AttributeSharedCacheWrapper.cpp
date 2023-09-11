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

#include <chaos/cu_toolkit/control_manager/AttributeSharedCacheWrapper.h>

using namespace chaos::cu::control_manager;
using namespace chaos::common::data::cache;

AttributeSharedCacheWrapper::AttributeSharedCacheWrapper(AbstractSharedDomainCache *_attribute_value_shared_cache):
attribute_value_shared_cache(_attribute_value_shared_cache){
	
}

AttributeSharedCacheWrapper::~AttributeSharedCacheWrapper() {
	
}

void AttributeSharedCacheWrapper::setOutputDomainAsChanged() {
	AttributeCache& attribute_setting = attribute_value_shared_cache->getSharedDomain(DOMAIN_OUTPUT);
	attribute_setting.markAllAsChanged();
}

void AttributeSharedCacheWrapper::setInputDomainAsChanged() {
	AttributeCache& attribute_setting = attribute_value_shared_cache->getSharedDomain(DOMAIN_INPUT);
	attribute_setting.markAllAsChanged();
}

// Set the value for a determinated variable in a determinate domain
void AttributeSharedCacheWrapper::setOutputAttributeValue(const std::string& attribute_name,
														  void * value,
														  uint32_t size,chaos::AllocationStrategy copy) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->setAttributeValue(DOMAIN_OUTPUT,
													attribute_name,
													value,
													size,copy);
}

// Set the value for a determinated variable in a determinate domain
void AttributeSharedCacheWrapper::setOutputAttributeValue(VariableIndexType attribute_index,
																void * value,
																uint32_t size,chaos::AllocationStrategy copy) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->setAttributeValue(DOMAIN_OUTPUT,
												   attribute_index,
												   value,
												   size,copy);
}
void AttributeSharedCacheWrapper::setOutputAttributeValue(const std::string& attribute_name,
											 chaos::common::data::Buffer * buf,chaos::AllocationStrategy copy){
												 CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->setAttributeValue(DOMAIN_OUTPUT,
												   attribute_name,
												   buf,copy);

}

void AttributeSharedCacheWrapper::setInputAttributeValue(const std::string& attribute_name,
																void * value,
																uint32_t size) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->setAttributeValue(DOMAIN_INPUT,
												   attribute_name,
												   value,
												   size);
}
bool AttributeSharedCacheWrapper::setOutputAttributeNewSize(const std::string& attribute_name,
															uint32_t new_size,
                                                            bool clear_mem) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	AttributeCache& attribute_setting = attribute_value_shared_cache->getSharedDomain(DOMAIN_OUTPUT);
	return attribute_setting.setNewSize(attribute_setting.getIndexForName(attribute_name),
                                        new_size,
                                        clear_mem);
}
bool AttributeSharedCacheWrapper::exist(SharedCacheDomain domain,const std::string&name){
		CHAOS_ASSERT(attribute_value_shared_cache)

		AttributeCache& attribute_setting = attribute_value_shared_cache->getSharedDomain(domain);
		return attribute_setting.hasName(name);
}

bool AttributeSharedCacheWrapper::setOutputAttributeNewSize(VariableIndexType attribute_index,
                                                            uint32_t new_size,
                                                            bool clear_mem) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	AttributeCache& attribute_setting = attribute_value_shared_cache->getSharedDomain(DOMAIN_OUTPUT);
	return attribute_setting.setNewSize(attribute_index,
                                        new_size,
                                        clear_mem);
}

ChaosSharedPtr<SharedCacheLockDomain> AttributeSharedCacheWrapper::getLockOnOutputAttributeCache(bool write_lock) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	return attribute_value_shared_cache->getLockOnDomain(DOMAIN_OUTPUT, write_lock);
}

// Get the index of the changed attribute
void AttributeSharedCacheWrapper::getChangedInputAttributeIndex(std::vector<VariableIndexType>& changed_index) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->getChangedAttributeIndex(DOMAIN_INPUT,
														   changed_index);
}

void AttributeSharedCacheWrapper::resetChangedInputIndex() {
	AttributeCache& attribute_setting = attribute_value_shared_cache->getSharedDomain(DOMAIN_INPUT);
	attribute_setting.resetChangedIndex();
}

// Return the names of all variabl einto a determinated domain
void AttributeSharedCacheWrapper::getAttributeNames(SharedCacheDomain domain,
													std::vector<std::string>& names) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->getAttributeNames(domain, names);
}
//! Add a new variable
void AttributeSharedCacheWrapper::addCustomAttribute(const std::string&  name,
										 const chaos::common::data::CDataWrapper& value){
									CHAOS_ASSERT(attribute_value_shared_cache);
									 std::string svalue=value.getCompliantJSONString();

	attribute_value_shared_cache->addAttribute(DOMAIN_CUSTOM,
											   name,
											   svalue.size()+1,
											   chaos::DataType::TYPE_JSON);		 
										 }
// Add a new variable
void AttributeSharedCacheWrapper::addCustomAttribute(const std::string&  name,
													 uint32_t max_size,
													 chaos::DataType::DataType type) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->addAttribute(DOMAIN_CUSTOM,
											   name,
											   max_size,
											   type);
}
void AttributeSharedCacheWrapper::setCustomAttributeValue(const std::string& attribute_name,
											 const chaos::common::data::CDataWrapper& value){
												 std::string svalue=value.getCompliantJSONString();
												 LDBG_<<"Set "<<attribute_name<<" to:"<<svalue;
												 setCustomAttributeValue(attribute_name,(void*)svalue.c_str(),(uint32_t)svalue.size()+1);

											 }

chaos::common::data::CDWUniquePtr AttributeSharedCacheWrapper::getCDValue(SharedCacheDomain domain){
	chaos::common::data::CDWUniquePtr p(new chaos::common::data::CDataWrapper());
	std::vector<std::string> names;
	attribute_value_shared_cache->getAttributeNames(domain, names);
	for(std::vector<std::string>::iterator i=names.begin();i!=names.end();i++){
		AttributeValue *value_setting = attribute_value_shared_cache->getAttributeValue(domain, *i);
		if(value_setting){
			value_setting->writeToCDataWrapper(*p.get());
		}

	}
	
	return p;
}
chaos::common::data::CDWUniquePtr AttributeSharedCacheWrapper::getCDValue(SharedCacheDomain domain,const std::string& attribute_name){
	
	AttributeValue *value_setting = attribute_value_shared_cache->getAttributeValue(domain, attribute_name);
	if(value_setting){
			chaos::common::data::CDWUniquePtr p(new chaos::common::data::CDataWrapper());

			value_setting->writeToCDataWrapper(*p.get());
			return p;

	}

	
	return chaos::common::data::CDWUniquePtr();
}

// Set the value for a determinated variable in a determinate domain
void AttributeSharedCacheWrapper::setCustomAttributeValue(const std::string& attribute_name,
																void * value,
																uint32_t size) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->setAttributeValue(DOMAIN_CUSTOM,
												   attribute_name,
												   value,
												   size);
}

// Set the value for a determinated variable in a determinate domain
void AttributeSharedCacheWrapper::setCustomAttributeValue(VariableIndexType attribute_index,
																  void * value,
																  uint32_t size) {
	CHAOS_ASSERT(attribute_value_shared_cache)
	attribute_value_shared_cache->setAttributeValue(DOMAIN_CUSTOM,
												   attribute_index,
												   value,
												   size);
}
chaos::DataType::DataType AttributeSharedCacheWrapper::getType(SharedCacheDomain domain,const std::string&name){
		AttributeCache& attribute_setting = attribute_value_shared_cache->getSharedDomain(domain);
		return attribute_setting.getType(name);

}

void AttributeSharedCacheWrapper::setCustomDomainAsChanged() {
	AttributeCache& cached = attribute_value_shared_cache->getSharedDomain(DOMAIN_CUSTOM);
	cached.markAllAsChanged();

}

ChaosSharedPtr<SharedCacheLockDomain> AttributeSharedCacheWrapper::getLockOnCustomAttributeCache(bool write_lock) {
	return 	attribute_value_shared_cache->getLockOnDomain(DOMAIN_CUSTOM, write_lock);
}

ChaosSharedPtr<SharedCacheLockDomain> AttributeSharedCacheWrapper::getReadLockOnInputAttributeCache() {
	return 	attribute_value_shared_cache->getLockOnDomain(DOMAIN_INPUT, false);
}
