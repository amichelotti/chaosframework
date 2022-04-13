#include "HttpStreamManager.h"
#include "impl/mjpeg_streamer.hpp"
#include <chaos/common/configuration/GlobalConfiguration.h>

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

               if(init_data){
                   port = atoi((const char*)init_data);
               }
               if(streamer){
                 streamer->start(port);
               }
               std::stringstream ss;
               ss<<chaos::GlobalConfiguration::getInstance()->getLocalServerAddress()<<":"<<port<<"/";
               rootpath=ss.str();
                DIODMC_INFO<<"Server STARTED "<<rootpath;
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