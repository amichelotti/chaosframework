/**
 * @brief Abstraction for message producer/subscribe
 * 
 */
#ifndef __MESSAGE_PSRDKAFKA_H__
#define __MESSAGE_PSRDKAFKA_H__
#include "../../MessagePublishSubscribeBase.h"
#include <librdkafka/rdkafka.h>

namespace chaos {
    namespace common {
        namespace message {
            class MessagePSRDKafka: public MessagePublishSubscribeBase {

                protected:
                rd_kafka_conf_t* conf;
                rd_kafka_topic_conf_t* topic_conf;
                std::string defkey;
                public:

                MessagePSRDKafka(const std::string& clientid);
                MessagePSRDKafka(const std::string& clientid,const std::string& k);
                ~MessagePSRDKafka();
                
                int setOption(const std::string&key,const std::string& value);

                int applyConfiguration();




            };
        }
        }
        }
#endif