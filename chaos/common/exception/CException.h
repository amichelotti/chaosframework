/*
 *	ControlException.h
 *	!CHOAS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2012 INFN, National Institute of Nuclear Physics
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
#ifndef ControlException_H
#define ControlException_H
#include <string>
#include <cstring>
#include <exception>
#include <sstream>

using namespace std;
namespace chaos{
    /*
     Base class for exception in control system library
     */
    class CException : public std::exception {
    public:
        //identify the number for the error
        int errorCode;
        //describe the error that occour
        string errorMessage;
        //identify the domain(ControlUnit, DataManager, ....)
        string errorDomain;
        
        explicit CException(int eCode, const char * eMessage,  const char * eDomain):errorCode(eCode),
        errorMessage( eMessage, strlen( eMessage )),
        errorDomain( eDomain, strlen( eDomain )){};
        
        explicit CException(int eCode, std::string& eMessage,  std::string& eDomain):errorCode(eCode),
        errorMessage(eMessage),
        errorDomain(eDomain) {};
        
        virtual ~CException() throw() {};
        
        virtual const char* what() const throw() {
            std::stringstream ss;
            ss << "-----------Exception------------";\
            ss << "Domain:" << errorDomain;\
            ss << "Message:" << errorMessage;\
            ss << "Error Code;" << errorCode;\
            ss << "-----------Exception------------";
            return ss.str().c_str();
        }
    };
}
#endif
