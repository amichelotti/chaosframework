/*
 * Copyright 2021 INFN
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

#ifndef __CHAOSFramework__CE8768D_5BB0_4EF2_A186_7685A0C31B3F_QueryCursorRPC_h
#define __CHAOSFramework__CE8768D_5BB0_4EF2_A186_7685A0C31B3F_QueryCursorRPC_h

#include <chaos/common/chaos_types.h>
#include "QueryCursor.h"
#include <stdint.h>

namespace chaos {
    namespace common {
        namespace io {
            class IODirectIOPSMsgDriver;
            
            
#undef DEFAULT_PAGE_LEN         
#define DEFAULT_PAGE_LEN 1000
            class QueryCursorRPC :public QueryCursor{
                friend class IODirectIOPSMsgDriver;

            protected:
                
                QueryCursorRPC(const std::string& _query_id,
                            const std::string& _node_id,
                            uint64_t _start_ts,
                            uint64_t _end_ts,
                            uint32_t page_len=DEFAULT_PAGE_LEN);

                QueryCursorRPC(const std::string&                        _query_id,
                            const std::string&                        _node_id,
                            uint64_t                                  _start_ts,
                            uint64_t                                  _end_ts,
                            const ChaosStringSet&                     _meta_tags,
                            const ChaosStringSet&                     _projection_keys,
                            uint32_t                                  page_len = DEFAULT_PAGE_LEN);

                QueryCursorRPC(const std::string&                        _query_id,
                            const std::string&                        _node_id,
                            uint64_t                                  _start_ts,
                            uint64_t                                  _end_ts,
                            uint64_t                                  _sequid,
                            uint64_t                                  _runid,
                            uint32_t                                  page_len = DEFAULT_PAGE_LEN);

                QueryCursorRPC(const std::string& _query_id,
                            const std::string& _node_id,
                            uint64_t _start_ts,
                            uint64_t _end_ts,
                            uint64_t _sequid,
                            uint64_t _runid,
                            const ChaosStringSet& _meta_tags,
                            const ChaosStringSet&  _projection_keys,
                            uint32_t page_len=DEFAULT_PAGE_LEN);
                ~QueryCursorRPC();
                
                virtual int fetchData();
            };
            
        }
    }
}

#endif /* __CHAOSFramework__CE8768D_5BB0_4EF2_A186_7685A0C31B3F_QueryCursorRPC_h */
