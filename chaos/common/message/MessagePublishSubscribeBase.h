/**
 * @brief Abstraction for message producer/subscribe
 * 
 */
#ifndef __MESSAGE_PUBLISHSUBSCRIBE_H__
#define __MESSAGE_PUBLISHSUBSCRIBE_H__
#include <set>
#include <string>
#include <chaos/common/data/CDataWrapper.h>
#include <boost/lockfree/queue.hpp>
#define  MSG_TIMEOUT_MS 10000
namespace chaos {
    namespace common {
        namespace message {
            typedef struct ele {std::string key;uint32_t off;uint32_t par;chaos::common::data::CDWUniquePtr cd;} ele_t;
           // typedef boost::lockfree::queue<ele_t*> msg_queue_t;
            typedef ChaosUniquePtr<ele_t> ele_uptr_t;
            //typedef std::vector<chaos::common::data::CDWShrdPtr> msg_queue_t;
            
          /*  template <class T>
            struct funchandler{
                typedef void (T::*msgHandler)(const chaos::common::message::ele_t&) ;

            };
*/
            typedef  boost::function<void(chaos::common::message::ele_t&)> msgHandler;

            class MessagePublishSubscribeBase {


                public:
                
           //     typedef void (*msgHandler)(const ele_t&);
                typedef struct msgstats {
                    uint64_t counter;
                    uint64_t oks;
                    uint64_t errs;
                    int last_err;

                    uint32_t offset;
                    std::string key;
                    std::string broker;
                    uint32_t partition;

                    msgstats():counter(0),oks(0),errs(0),last_err(0),offset(0),partition(0){}
                } msgstats_t;
                enum eventTypes{
                    ONDELIVERY,
                    ONARRIVE,
                    ONERROR
                };
                enum msgOpt{
                    MSG_COPY=0,
                    MSG_NOCOPY=1,
                    MSG_SYNCH=2
                };
                protected:
                bool        running;
                msgOpt msg_opt;
                std::string errstr;
                std::set<std::string> servers;
                std::string id;
                std::map< eventTypes,msgHandler> handlers;
                std::map< std::string,msgHandler> topic_handlers;

                msgstats_t stats;
                boost::atomic<bool>   data_ready;
                ChaosMutex mutex_cond;
                ChaosConditionVariable cond;
                boost::thread th;
                void thfunc();
                ChaosRecursiveMutex io;
                uint64_t    counter,oks,errs;

                public:
                MessagePublishSubscribeBase(const std::string& _id):data_ready(false),running(false),id(_id),msg_opt(MSG_COPY){};

                msgstats_t getStats() const{ return stats;}

                virtual ~MessagePublishSubscribeBase();
                void addServer(const std::string&url);

                /**
                 * @brief Add an handler to the message
                 * 
                 * @param ev 
                 */
                int addHandler(eventTypes ev,msgHandler cb,bool add=true);
                int addHandler(const std::string& ev,msgHandler cb,bool add=true);
                /**
                 * @brief Enable synchronous if supported
                 * 
                 * @param sync enable/disable
                 */
        
                void setMsgOpt(msgOpt opt){msg_opt=opt;}
                msgOpt getMsgOpt()const {return msg_opt;}
                /**
                 * @brief Add an handler to the message
                 * 
                 * @param ev 
                 */
                 bool handlersEmpty(){
                    return handlers.empty();
                }
                /**
                 * @brief library specific options
                 * 
                 * @param key option name
                 * @param value value
                 * @return int 
                 */
                virtual int setOption(const std::string& key,const std::string&value);

 /**
                 * @brief apply configuration 
                 * 
                 * @return 0 if success
                 */
                virtual int applyConfiguration();

                /**
                 * @brief Wait for the completion of a async operation
                 * 
                 * @param timeout timeout in ms
                 * @return int 0 if success, negative if error
                 */
                virtual int waitCompletion(const uint32_t timeout_ms=MSG_TIMEOUT_MS);


                /**
                 * @brief Delete a key
                 * 
                 * @param key key to remove
                 * @return int 0 on success
                 */
                virtual int deleteKey(const std::string& key);

                /**
                 * @brief Create a key
                 * 
                 * @param key key to add
                 * @return int 0 on success
                 */
                virtual int createKey(const std::string& key);

                /**
                 * @brief Set Maximum message size
                 * 
                 * @param size set the maximum message size if supported
                 * @return int 0 on success
                 */
                virtual int setMaxMsgSize(const int size);

                void start();
                void stop();
                virtual void poll();
                std::string getLastError(){return errstr;}
            };
        }
        }
        }
#endif