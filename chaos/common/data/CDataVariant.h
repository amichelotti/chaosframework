/*
 *	CDataVariant.h
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by Claudio Bisegni.
 *
 *    	Copyright 30/03/16 INFN, National Institute of Nuclear Physics
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

#ifndef __CHAOSFramework__CDataVariant_h
#define __CHAOSFramework__CDataVariant_h

#include <chaos/common/chaos_constants.h>
#include <chaos/common/data/CDataBuffer.h>
#include <chaos/common/data/CDataWrapper.h>

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#define CHAOS_VARIANT_DEFINE_VISITOR_WITH_NAME_TYPE(n,t)\
class n ## _visitor:\
public boost::static_visitor< t > {\
public:\
    t operator()(const bool bv) const;\
    t operator()(const int32_t i32v) const;\
    t operator()(const uint32_t ui32v) const;\
    t operator()(const int64_t i64v) const;\
    t operator()(const uint64_t ui64v) const;\
    t operator()(double dv) const;\
    t operator()(const std::string& str) const;\
    t operator()(const boost::shared_ptr<chaos::common::data::CDataBuffer>& buffer) const;\
    t operator()(const boost::shared_ptr<chaos::common::data::CDataWrapper>& buffer) const;\
};

#define CHAOS_VARIANT_DEFINE_VISITOR_WITH_TYPE(t) CHAOS_VARIANT_DEFINE_VISITOR_WITH_NAME_TYPE(t,t)

namespace chaos {
    namespace common {
        namespace data {
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_TYPE(bool);
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_TYPE(int32_t);
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_TYPE(uint32_t);
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_TYPE(int64_t);
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_TYPE(uint64_t);
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_TYPE(double);
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_NAME_TYPE(string, std::string);
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_NAME_TYPE(CDataBuffer, boost::shared_ptr<chaos::common::data::CDataBuffer>);
            CHAOS_VARIANT_DEFINE_VISITOR_WITH_NAME_TYPE(CDataWrapper, boost::shared_ptr<chaos::common::data::CDataWrapper>);

            
            /*!
             * Chaos variant implementation that host all dataset CHAOS data type
             */
            class CDataVariant {
                DataType::DataType type;
                boost::variant<int32_t,
                uint32_t,
                int64_t,
                uint64_t,
                double,
                bool,
                std::string,
                boost::shared_ptr<chaos::common::data::CDataBuffer>,
				boost::shared_ptr<chaos::common::data::CDataWrapper> > _internal_variant;
            public:
                explicit CDataVariant(DataType::DataType _type,
                                      const void *_value_pointer,
                                      uint32_t _value_size);
                explicit CDataVariant(int32_t int32_value);
                explicit CDataVariant(uint32_t int32_value);
                explicit CDataVariant(int64_t int64_value);
                explicit CDataVariant(uint64_t int64_value);
                explicit CDataVariant(double double_value);
                explicit CDataVariant(bool boolvalue);
                explicit CDataVariant(const std::string& string_value);
                explicit CDataVariant(const char * string_value);
                //! take the ownership of the object
                explicit CDataVariant(CDataBuffer *buffer_value);
                explicit CDataVariant(CDataWrapper *buffer_value);

                CDataVariant(const CDataVariant& to_copy);

                CDataVariant();
                
                CDataVariant& operator=(const CDataVariant& arg);
                
                DataType::DataType getType() const;
                bool isValid() const;
                
                int32_t asInt32() const;
                operator int32_t() const;
                
                uint32_t asUInt32() const;
                operator uint32_t() const;
                
                int64_t asInt64() const;
                operator int64_t() const;
                
                uint64_t asUInt64() const;
                operator uint64_t() const;
                
                double asDouble() const;
                operator double() const;
                
                bool asBool() const;
                operator bool() const;
                
                const std::string asString() const;
                operator std::string() const;
                
                const chaos::common::data::CDataBuffer *const asCDataBuffer() const;
                operator const chaos::common::data::CDataBuffer *() const;
                
                boost::shared_ptr<CDataBuffer> asCDataBufferShrdPtr();
                operator boost::shared_ptr<CDataBuffer>();
                
                const chaos::common::data::CDataWrapper *const asCDataWrapper() const;
                operator const chaos::common::data::CDataWrapper *const() const;
                
                boost::shared_ptr<CDataWrapper> asCDataWrapperShrdPtr();
                operator boost::shared_ptr<CDataWrapper>();
                
            };
        }
    }
}

#endif /* __CHAOSFramework__CDataVariant_h */
