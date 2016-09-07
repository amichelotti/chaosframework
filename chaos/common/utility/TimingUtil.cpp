/*
 *	TimingUtil.cpp
 *
 *	!CHAOS [CHAOSFramework]
 *	Created by bisegni.
 *
 *    	Copyright 08/08/16 INFN, National Institute of Nuclear Physics
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

#include <chaos/common/utility/TimingUtil.h>

using namespace chaos::common::utility;

const char* TimingUtil::formats[]={
  "%Y-%m-%dT%H:%M:%S.%fZ",
  "%Y-%m-%d %H:%M:%S.%fZ",
  "%Y-%m-%d %H:%M:%SZ",
  "%Y-%m-%dT%H:%M:%SZ",
  "%Y-%m-%d %H:%MZ",
  "%Y-%m-%dT%H:%MZ",
  "%Y-%m-%d %HZ",
  "%Y-%m-%dT%HZ",
  "%Y-%m-%dZ",
  "%Y-%mZ",
  "%YZ"
};

const size_t TimingUtil::formats_n = 11;
