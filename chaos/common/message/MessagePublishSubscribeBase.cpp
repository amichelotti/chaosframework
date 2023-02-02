#include "MessagePublishSubscribeBase.h"
#include <chaos/common/global.h>
#define MRDAPP_ INFO_LOG(MessagePublishSubscribeBase)
#define MRDDBG_ DBG_LOG(MessagePublishSubscribeBase)
#define MRDERR_ ERR_LOG(MessagePublishSubscribeBase)

namespace chaos {
    namespace common {
        namespace message {

            MessagePublishSubscribeBase::~MessagePublishSubscribeBase(){
                MRDDBG_ << "destroying";
                th.interrupt();
                stop();
            
            }
                void MessagePublishSubscribeBase::addServer(const std::string&url){
                    std::lock_guard<std::recursive_mutex> ll(io);

                    MRDDBG_<<"["<<servers.size()<<"] adding server:"<<url;
                    servers.insert(url);
                }

                int MessagePublishSubscribeBase::setOption(const std::string&key,const std::string& value){
                    MRDDBG_<<"NOT IMPLEMENTED";

                    return 0;
                }


        /* int MessagePublishSubscribeBase::applyConfiguration(){
             if(impl==NULL){
                MRDERR_<<"NOT a valid implementation";
                return -5;

            }
             return impl->applyConfiguration();
         }*/
         int MessagePublishSubscribeBase::addHandler(eventTypes ev,msgHandler cb,bool add){
                    if(add){
                     MRDDBG_<<"Register handler "<<ev<<" @"<<std::hex<<cb;

                        handlers[ev]=cb;
                    } else if(handlers.count(ev)){
                        MRDDBG_<<"UNRegister handler "<<ev;

                        handlers.erase(ev);
                    }
                    return 0;
                }
        int MessagePublishSubscribeBase::addHandler(const std::string& ev,msgHandler cb,bool add){
                    std::string key=ev;
                     if(key.size()==0){
                        return -1;
                    }
                    std::replace(key.begin(), key.end(), '/', '.');
                    std::replace(key.begin(), key.end(), ':', '.');
                    if(add){

                        topic_handlers[key]=cb;
                        MRDDBG_<<topic_handlers.size()<<" + Register handler on key:"<<ev<<" topic:"<<key<<" @"<<std::hex<<cb;

                    } else if(topic_handlers.count(key)){
                        topic_handlers.erase(key);

                        MRDDBG_<<topic_handlers.size()<<" - UNRegister handler on key:"<<ev<<" topic:"<<key;

                    }
                    return 0;
                }
          int MessagePublishSubscribeBase::applyConfiguration(){
                MRDDBG_<<"NOT IMPLEMENTED";

            return 0;
            }
         int MessagePublishSubscribeBase::waitCompletion(const uint32_t timeout_ms){
                ChaosUniqueLock guard(mutex_cond);
                MRDDBG_<<"wating operation";
                if(data_ready) return stats.last_err;
                if(!CHAOS_WAIT_MS(cond,guard,timeout_ms)){
                MRDERR_<<"Timeout";
                return -100;
                }

                return stats.last_err; 
            }

        int MessagePublishSubscribeBase::deleteKey(const std::string& key){
           MRDDBG_<<"NOT IMPLEMENTED";

            return 0;
         
        }
        int MessagePublishSubscribeBase::setMaxMsgSize(const int size){
           MRDDBG_<<"NOT IMPLEMENTED";

            return 0;
          
        }
         void MessagePublishSubscribeBase::thfunc(){
                running =true;
                MRDDBG_<<"Start polling";
                while(running){
                    poll();
                }
                MRDDBG_<<"End polling";
            }
    
         int MessagePublishSubscribeBase::createKey(const std::string& key){
              MRDDBG_<<"NOT IMPLEMENTED";

            return 0;

         }
         void MessagePublishSubscribeBase::poll(){
             MRDDBG_<<"NOT IMPLEMENTED";

         }

         void MessagePublishSubscribeBase::start(){
             std::lock_guard<std::recursive_mutex> ll(io);
            if(running==true){
                 MRDDBG_<<"Already running";
                return;
            }
             running=true;
             boost::thread(&MessagePublishSubscribeBase::thfunc,this);

         }
        void MessagePublishSubscribeBase::stop(){
            std::lock_guard<std::recursive_mutex> ll(io);
            if(running==false){
                MRDDBG_<<"Already stopped";

                th.join();
                return;
            }
            MRDDBG_<<"Stopping";

            running=false;
            th.join();
        }
                

        }
        }
        }