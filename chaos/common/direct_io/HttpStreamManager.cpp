#include "HttpStreamManager.h"
#include "impl/mjpeg_streamer.hpp"
#include <chaos/common/configuration/GlobalConfiguration.h>
//#include <boost/algorithm/string.hpp>
#include <chaos/common/ChaosCommon.h>
#define DIODMC_INFO INFO_LOG(HttpStreamManager)
#define DIODMC_DBG_ DBG_LOG(HttpStreamManager)
#define DIODMC_ERR_ ERR_LOG(HttpStreamManager)

namespace chaos {
	namespace common {
		namespace direct_io {
        HttpStreamManager::HttpStreamManager(){
            streamer = new nadjieb::MJPEGStreamer();


        }
        HttpStreamManager::~HttpStreamManager(){
            deinit();
        }

           void HttpStreamManager::init(void *init_data){
               int port=STREAMER_PORT;
               int workers=1;
                std::vector<std::string> server_desc_tokens;
               if(init_data){
                   std::string tos(((const char*)init_data));

                   	//boost::algorithm::split(server_desc_tokens, tos, boost::algorithm::is_any_of(":"), boost::algorithm::token_compress_on);
                    server_desc_tokens=chaos::split( tos, ":", true);
                    port=atoi(server_desc_tokens[1].c_str());
                    workers=atoi(server_desc_tokens[2].c_str());
                 //  sscanf((const char*)init_data,"%s:%d:%d",ip,&port,&workers);
               }
               if(server_desc_tokens.size()!=3){
                   delete streamer;
                   streamer=NULL;
                   throw chaos::CException(-1, "Not configured properly", __FUNCTION__);
               }
               if(streamer){
                 
                 streamer->start(port,workers);
               }
               std::stringstream ss;

               ss<<server_desc_tokens[0]<<":"<<port<<"/";
               rootpath=ss.str();
                DIODMC_INFO<<"Server STARTED "<<rootpath <<" with "<<workers<<" workers";
           }
            void HttpStreamManager::publish(const std::string&path,const std::string&data){

                streamer->publish("/"+path,data);
             //   DIODMC_DBG_<<"Publishing  "<<(rootpath+path);

            }

            void HttpStreamManager::deinit(){
                if(streamer){
                    DIODMC_INFO<<"Server STOPPED "<<rootpath;

                    streamer->stop();

                    delete streamer;
                    streamer=NULL;
                }
            }   
        
        }}}