#include "MessagePSRDKafka.h"
#include <chaos/common/global.h>
#define MRDAPP_ INFO_LOG(MessagePSRDKafka)
#define MRDDBG_ DBG_LOG(MessagePSRDKafka)
#define MRDERR_ ERR_LOG(MessagePSRDKafka)

namespace chaos {
namespace common {
namespace message {
namespace kafka {
namespace rdk {

MessagePSRDKafka::~MessagePSRDKafka() {
}


void MessagePSRDKafka::poll(){
    sleep(1);
}
MessagePSRDKafka::MessagePSRDKafka():MessagePublishSubscribeBase("kafka-rdk"),rk(NULL) {
  conf       = rd_kafka_conf_new();
  topic_conf = rd_kafka_topic_conf_new();
}

int MessagePSRDKafka::setOption(const std::string& key, const std::string& value) {
  char errstr[512];
  MRDDBG_<<"set \""<<key<<"\"="<<value;
  if (rd_kafka_conf_set(conf, key.c_str(), value.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
    MRDERR_ << "error:" << errstr;

    return -2;
  }
  return 0;
}
int MessagePSRDKafka::setMaxMsgSize(const int size){
  char sinteger[256];
  sprintf(sinteger,"%d",size);
  return setOption("message.max.bytes",sinteger);
}

int MessagePSRDKafka::init(std::set<std::string>& servers) {
  char hostname[128];
  char errstr[512];

  if (servers.size() == 0) {
    MRDERR_ << " no server specified!";
    return -4;
  }
  if (gethostname(hostname, sizeof(hostname))) {
    MRDERR_ << "Failed to lookup hostname";
    return -1;
  }

  if (rd_kafka_conf_set(conf, "client.id", hostname, errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
    MRDERR_ << "error:" << errstr;

    return -2;
  }
  std::stringstream ss;

  for (std::set<std::string>::iterator i = servers.begin(); i != servers.end(); i) {
    ss << *i;
    if ((++i) != servers.end()) {
      ss << ",";
    }
  }

  if (rd_kafka_conf_set(conf, "bootstrap.servers", ss.str().c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
    MRDERR_ << "setting bootstrap servers:" << errstr;
    return -4;
  }

  if (rd_kafka_topic_conf_set(topic_conf, "acks", "all", errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
    MRDERR_ << "setting topic conf:" << errstr;

    return -5;
  }
  return 0;
}



  
  int MessagePSRDKafka::deleteKey(const std::string& key){
    return 0;

  }
  int MessagePSRDKafka::createKey(const std::string& key){
    return 0;

  }
  
  int MessagePSRDKafka::getOffset(const std::string& key,uint32_t &off,int type,int par){
    return 0;

  }

}  // namespace rdk
}  // namespace kafka
}  // namespace message
}  // namespace common
}  // namespace chaos