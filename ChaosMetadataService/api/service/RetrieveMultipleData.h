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

#ifndef __CHAOSFramework__D12C0AE_833D_4BC5_B5F6_76CF1D4950A0_RetrieveMultipleData_h
#define __CHAOSFramework__D12C0AE_833D_4BC5_B5F6_76CF1D4950A0_RetrieveMultipleData_h

#include "../AbstractApi.h"
#include <chaos/common/direct_io/channel/DirectIODeviceChannelGlobal.h>

namespace chaos {
    namespace metadata_service {
        namespace api {
            namespace service {
                class RetrieveMultipleData:
                    public AbstractApi {
                    protected:
                    public:
                     RetrieveMultipleData();
                    ~RetrieveMultipleData();
                    chaos::common::data::CDWUniquePtr execute(chaos::common::data::CDWUniquePtr api_data);
                    int execute(const ChaosStringVector& keys,chaos::common::data::VectorCDWShrdPtr& result);

};
                
            }
        }
    }
}

#endif /* __CHAOSFramework__D12C0AE_833D_4BC5_B5F6_76CF1D4950A0_RetrieveMultipleData_h */
