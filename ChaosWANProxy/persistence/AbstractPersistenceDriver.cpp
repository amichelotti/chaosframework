//
//  AbstractPersistenceDriver.cpp
//  CHAOSFramework
//
//  Created by Claudio Bisegni on 14/01/15.
//  Copyright (c) 2015 INFN. All rights reserved.
//

#include "AbstractPersistenceDriver.h"

using namespace chaos::wan_proxy::persistence;

AbstractPersistenceDriver::AbstractPersistenceDriver(const std::string& name):
NamedService(name) {
	
}

AbstractPersistenceDriver::~AbstractPersistenceDriver() {
	
}