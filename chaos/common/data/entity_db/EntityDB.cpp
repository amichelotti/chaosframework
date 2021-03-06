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
#include "EntityDB.h"
#include <chaos/common/data/entity/Entity.h>

using namespace chaos;
using namespace chaos::edb;

/*!
 Default constructor
 */
EntityDB::EntityDB() {
}

/*!
 Default destructor
 */
EntityDB::~EntityDB() {
}

/*!
 Allocate and return a new instance of an Entity class
 */
entity::Entity* EntityDB::getNewEntityInstance(KeyIdAndValue& keyInfo) {
    //atomic_int_type instance = atomic_increment(&entityInstanceSequence);
    entity::Entity *result = new entity::Entity(this);
    if(result) result->setEntityKeyAndInfo(keyInfo);
    return result;
}