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

#ifndef SingletonCW_H
#define SingletonCW_H

#include <boost/thread/once.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <chaos/common/data/CDataWrapper.h>
//#include <chaos/common/global.h>

namespace chaos {
    namespace common {
        namespace utility {
            
            /*
             * Utility class for singleton find here: http://www.boostcookbook.com/Recipe:/1235044
             */
            template<class T>
            class SingletonCW:
            private boost::noncopyable {
            public:
               
                static T *getInstance(const chaos::common::data::CDataWrapper&conf) {
                    //static T singletonInstance;
                    call_once(boost::bind(&_singletonInit, conf), flag);
                    return t;
                }
                static T *getInstance() {
                    //static T singletonInstance;
                    call_once(_singletonInit0, flag);
                    return t;
                }  
                static void _singletonInit0() {
                    if(t==0){
                        t= new T();
                    }
                }           
                static void _singletonInit(const chaos::common::data::CDataWrapper&conf) {
                    if(t==0){
                        t= new T(conf);
                    }
                }
                virtual ~SingletonCW(){if(t) delete t;t=0L;}
            protected:
                static T*  t;
                SingletonCW(){}
            private:
                static boost::once_flag flag;
                // Stop the compiler generating methods of copy the object
                SingletonCW(SingletonCW const& copy);            // Not Implemented
                SingletonCW& operator=(SingletonCW const& copy); // Not Implemented
            };
            
            template<class T> T* SingletonCW<T>::t=0L;
            template<class T> boost::once_flag SingletonCW<T>::flag = BOOST_ONCE_INIT;
        }
    }
}
#endif
