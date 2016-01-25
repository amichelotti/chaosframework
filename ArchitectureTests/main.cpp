//
//  main.cpp
//  EchitectureTests
//
//  Created by Claudio Bisegni on 28/05/14.
//  Copyright (c) 2014 INFN. All rights reserved.
//

#include "utility/HashMapTest.h"
#include "network/FeederTest.h"
#include "thread/ObjectQueueTest.h"
#include "thread/ObjectPriorityQueueTest.h"
#include "queue/PriorityTest.h"
#include <cassert>
#include <chaos/common/property/PropertyCollector.h>

using namespace chaos;

class TestProp:
public chaos::common::property::PropertyCollector {
    
    void setPeropertyOne(int _new_property_one) {
        property_one = _new_property_one;
    }
    int getPeropertyOne() {
        return property_one;
    }
public:
    DECLARE_CHAOS_PROPERTY(TestProp, int, CHAOS_PROPERTY_READ_WRITE, property_one)

    TestProp() {
        REGISTER_CHAOS_PROPERTY("TestProp", property_one, &TestProp::setPeropertyOne, &TestProp::getPeropertyOne, "Get the property one")
    }
};

int main(int argc, const char * argv[]) {
    
    TestProp tp;
    tp.property_one = 34;
    int i = tp.property_one;
    std::string i_str =  tp.property_one;
    i_str = tp.getSectionPropertyStrValue("TestProp", "property_one");
    tp.setSectionPropertyStrValue("TestProp",
                          "property_one",
                          "100");
    i = tp.property_one;
    i_str = tp.getSectionPropertyStrValue("TestProp", "property_one");

    tp.property_one = "1500";
    i = tp.property_one;
    
    chaos::common::pqueue::test::PriorityTest ptest;
    if(!ptest.test(50, 1000)) return 1;
    
    chaos::test::network::FeederTest fd;
    fd.test(100000);
    
    chaos::common::pqueue::test::ObjectQueueTest oqt;
    if(!(oqt.test(10, 100, 100, 0, true) == true)) return 1;
	
	chaos::common::pqueue::test::ObjectPriorityQueueTest opqt;
	if(!(oqt.test(10, 100, 100, 0, true) == true)) return 1;

	chaos::common::utility::test::HashMapTest hmt;
	if(!hmt.test(10, 10, 10)) return 1;
    
    return 0;
}

