/*
 *	DataPack.cpp
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 02/08/2017 INFN, National Institute of Nuclear Physics
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

#include <chaos_micro_unit_toolkit/data/DataPack.h>

using namespace chaos::micro_unit_toolkit;
using namespace chaos::micro_unit_toolkit::data;

DataPackUniquePtr DataPack::newFromBuffer(const char *data,
                                          const size_t data_len,
                                          bool *parsed) {
    DataPackUniquePtr new_dp(new DataPack());
    Json::Reader reader;
    bool parse_result = reader.parse(data,
                                     data+data_len,
                                     new_dp->root_json_object);
    if(parsed){*parsed = parse_result;}
    return new_dp;
}

DataPack::DataPack() {}

DataPack::DataPack(const Json::Value& src_root_json_object):
root_json_object(src_root_json_object){}

DataPack::~DataPack() {}

bool DataPack::hasKey(const std::string& key) {
    return root_json_object.isMember(key);
}

void DataPack::addBool(const std::string& key, bool value) {
    root_json_object[key] = Json::Value(value);
}
const bool DataPack::isBool(const std::string& key) const {
    return root_json_object[key].isBool();
}

const bool DataPack::getBool(const std::string& key) const {
    return root_json_object[key].asBool();
}
void DataPack::addInt32(const std::string& key,
                        int32_t value) {
    root_json_object[key] = Json::Value(value);
}
const bool DataPack::isInt32(const std::string& key) const {
    return root_json_object[key].isInt();
}
const int32_t DataPack::getInt32(const std::string& key) const {
    return root_json_object[key].asInt();
}
void DataPack::addInt64(const std::string& key,
                             int64_t value) {
    root_json_object[key] = Json::Value(value);
}
const bool DataPack::isInt64(const std::string& key) const {
    return root_json_object[key].isInt64();
}
const int64_t DataPack::getInt64(const std::string& key) const {
    return root_json_object[key].asInt64();
}
void DataPack::addDouble(const std::string& key,
                         double value) {
    root_json_object[key] = Json::Value(value);
}
const bool DataPack::isDouble(const std::string& key) const {
    return root_json_object[key].isDouble();
}
const double DataPack::getDouble(const std::string& key) const {
    return root_json_object[key].asDouble();
}
void DataPack::addString(const std::string& key,
                         const std::string& value) {
    root_json_object[key] = Json::Value(value);
}
const bool DataPack::isString(const std::string& key) const {
    return root_json_object[key].isString();
}
std::string DataPack::getString(const std::string& key) const {
    return root_json_object[key].asString();
}
void DataPack::addDataPack(const std::string& key,
                           DataPack& value) {
    root_json_object[key] = value.root_json_object;
}
const bool DataPack::isDataPack(const std::string& key) const {
    return root_json_object[key].isObject();
}
const DataPackUniquePtr DataPack::getDataPack(const std::string& key) const {
    return DataPackUniquePtr(new DataPack(root_json_object[key]));
}
void DataPack::createArrayForKey(const std::string& key) {
    root_json_object[key] = Json::Value(Json::ValueType::arrayValue);
}
const bool DataPack::isArray(const std::string& key) const {
    return root_json_object[key].isArray();
}
void DataPack::appendBool(const std::string& arr_key,
                          bool value) {
    root_json_object[arr_key].append(Json::Value(value));
}

void DataPack::appendInt32(const std::string& arr_key,
                           int32_t value) {
    root_json_object[arr_key].append(Json::Value(value));
}

void DataPack::appendInt64(const std::string& arr_key,
                           int64_t value) {
    root_json_object[arr_key].append(Json::Value(value));
}

void DataPack::appendDouble(const std::string& arr_key,
                            double value) {
    root_json_object[arr_key].append(Json::Value(value));
}

void DataPack::appendString(const std::string& arr_key,
                            const std::string& value) {
    root_json_object[arr_key].append(Json::Value(value));
}

void DataPack::appendDataPack(const std::string& arr_key, DataPack& value) {
    root_json_object[arr_key].append(value.root_json_object);
}

std::string DataPack::toString() {
    Json::StyledWriter w;
    return w.write(root_json_object);
}

std::string DataPack::toUnformattedString() {
    Json::FastWriter w;
    return w.write(root_json_object);
}