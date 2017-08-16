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

#ifndef CHAOSFramework_InetUtility_h
#define CHAOSFramework_InetUtility_h

#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <chaos/common/global.h>

#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace chaos {
    namespace common {
        namespace utility {
            //! Regular expression for check server hostname and port
            static const char * const ServerHostName = "[a-zA-Z0-9]+(.[a-zA-Z0-9]+)+:[0-9]{4,5}";
            //! Regular expression for check server ip and port
            static const char * const ServerIPAndPort = "\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b:[0-9]{4,5}";
            //! Regular expression for check server ip
            static const char * const ServerIP = "\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b";
            
            
            class InetUtility {
                
            public:
                
                struct InterfaceInfo {
                    std::string interface_name;
                    std::string address;
                    
                    InterfaceInfo(const std::string& _interface_name,
                                  const std::string& _address):
                    interface_name(_interface_name),
                    address(_address){}
                    
                    InterfaceInfo(const InterfaceInfo& _info):
                    interface_name(_info.interface_name),
                    address(_info.address){}
                };
                
                static void checkInterfaceName(std::vector<InterfaceInfo>& interface_infos,
                                               const std::string& interface_found,
                                               const std::string& interface_address) {
                    for(std::vector<InterfaceInfo>::iterator it = interface_infos.begin();
                        it != interface_infos.end();
                        it++){
                        if(boost::starts_with(interface_found, it->interface_name)) {
                            it->interface_name = interface_found;
                            it->address = interface_address;
                            break;
                        }
                    }
                }
                
                //! Network address scan
                /*!
                 This methdo will take care for network address scan process
                 */
                static std::string scanForLocalNetworkAddress(std::string _eth_interface_name = std::string()) {
                    std::string ip_port;
                    void * tmp_addr_ptr=NULL;
                    struct ifaddrs * if_addr_struct=NULL;
                    struct ifaddrs * ifa=NULL;
                    std::string eth_interface_name;
                    std::vector<InterfaceInfo> interface_infos;
                    
                    //chec if we have a prefered name to use
                    if(_eth_interface_name.size()) {
                        interface_infos.push_back(InterfaceInfo(_eth_interface_name, ""));
                    }
                    
                    //default names are ppend to the end of the set
                    interface_infos.push_back(InterfaceInfo("en",""));
                    interface_infos.push_back(InterfaceInfo("em",""));
                    interface_infos.push_back(InterfaceInfo("eth",""));
                    interface_infos.push_back(InterfaceInfo("tun",""));
                    interface_infos.push_back(InterfaceInfo("utun",""));
                    // loopback if no other found
                    interface_infos.push_back(InterfaceInfo("lo",""));
                    
                    LAPP_ << "Scan for local network interface and ip";
                    getifaddrs(&if_addr_struct);
                    
                    for (ifa = if_addr_struct; ifa != NULL; ifa = ifa->ifa_next) {
                        if(ifa ->ifa_addr){
                            if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
                                // is a valid IP4 Address
                                tmp_addr_ptr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                                char address_buffer[INET_ADDRSTRLEN];
                                inet_ntop(AF_INET, tmp_addr_ptr, address_buffer, INET_ADDRSTRLEN);
                                LAPP_ << "Interface " << ifa->ifa_name << " address " << address_buffer << "<- candidate to be chooses";
                                checkInterfaceName(interface_infos,
                                                   ifa->ifa_name,
                                                   address_buffer);
                            } else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
                                // is a valid IP6 Address
                                tmp_addr_ptr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
                                char address_buffer[INET6_ADDRSTRLEN];
                                inet_ntop(AF_INET6, tmp_addr_ptr, address_buffer, INET6_ADDRSTRLEN);
                                LAPP_ << "Interface " << ifa->ifa_name << " address " << address_buffer;
                            }
                        }
                    }
                    if (if_addr_struct!=NULL) freeifaddrs(if_addr_struct);
                    
                    //check the first priority address to get
                    for(std::vector<InterfaceInfo>::const_iterator it = interface_infos.begin();
                        it != interface_infos.end();
                        it++){
                        if(it->address.size()) {
                            ip_port = it->address;
                            LAPP_ << "The choosen address is:" << it->address << " his interface is:" << it->interface_name;
                            break;
                        }
                    }
                    //in case we don't have found anything fall to de loopback ip
                    if(ip_port.size() == 0) {
                        // no ip was found go to localhost
                        ip_port.assign("127.0.0.1");
                    }
                    
                    return ip_port;
                }
                
                //! Network port scan
                /*!
                 This methdo will take care for search nex available port starting from a base value
                 */
                
                static int scanForLocalFreePort(int basePort) {
                    int sockfd, portno;
                    struct sockaddr_in serv_addr;
                    portno = basePort;
                    int err = 0;
                    for (; err>=0;) {
                        sockfd = socket(AF_INET, SOCK_STREAM, 0);
                        if (sockfd < 0) return -1;
                        bzero((char *) &serv_addr, sizeof(serv_addr));
                        serv_addr.sin_family = AF_INET;
                        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
                        serv_addr.sin_port = htons(portno);
                        err = connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
                        close(sockfd);
                        if(err>=0)portno++;
                    }
                    
                    return portno;
                }
                
                static bool checkWellFormedHostPort(std::string host_port) {
                    boost::regex ServerIPRegExp(ServerIP);
                    return regex_match(host_port, ServerIPRegExp);
                }
                
                static bool checkWellFormedHostNamePort(std::string host_port) {
                    boost::regex ServerHostNameRegExp(ServerHostName);
                    return regex_match(host_port, ServerHostNameRegExp);
                }
                
                static bool checkWellFormedHostIpPort(std::string host_port) {
                    boost::regex ServerIPAndPortRegExp(ServerIPAndPort);
                    return regex_match(host_port, ServerIPAndPortRegExp);
                }
                
                static void queryDns(std::string hostname, std::vector<std::string>& resolved_endpoints) {
                    boost::asio::io_service io;
                    boost::asio::ip::tcp::resolver resolver(io);
                    boost::asio::ip::tcp::resolver::query q(hostname.c_str(), "");
                    boost::system::error_code error;
                    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(q, error);
                    boost::asio::ip::tcp::resolver::iterator end;
                    while (iter != end) {
                        resolved_endpoints.push_back(iter->endpoint().address().to_string());
                        iter++;
                    }
                }
                
                static std::string getHostname() {
                    char hostname[1024];
                    gethostname(hostname, 1024);
                    return std::string(hostname, strlen(hostname));
                }
#define		STRIP_TO_UI64(x)  boost::asio::ip::address_v4::from_string(x)
#define		UI64_TO_STRIP(i)  boost::asio::ip::address_v4(i).to_string()
            };
        }
    }
}
#endif
