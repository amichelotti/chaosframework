#include "MessagePSDriver.h"
#define MRDAPP_ INFO_LOG(MessagePSDriver)
#define MRDDBG_ DBG_LOG(MessagePSDriver)
#define MRDERR_ ERR_LOG(MessagePSDriver)
#include <chaos/common/configuration/GlobalConfiguration.h>
#ifdef KAFKA_RDK_ENABLE
#include "impl/kafka/rdk/MessagePSKafkaProducer.h"
#include "impl/kafka/rdk/MessagePSRDKafkaConsumer.h"

#endif
#ifdef KAFKA_ASIO_ENABLE
#include "impl/kafka/asio/MessagePSKafkaAsioConsumer.h"
#include "impl/kafka/asio/MessagePSKafkaAsioProducer.h"

#endif
#include <chaos/common/global.h>

namespace chaos {
namespace common {
namespace message {

ChaosMutex                           MessagePSDriver::io;
std::map<std::string, producer_uptr_t> MessagePSDriver::producer_drv_m;
std::map<std::string, consumer_uptr_t> MessagePSDriver::consumer_drv_m;
producer_uptr_t                        MessagePSDriver::getNewProducerDriver(const std::string& drvname, const std::string& k) {
  producer_uptr_t ret;
#ifdef KAFKA_RDK_ENABLE

  if ((drvname == "KAFKA-RDK") || (drvname == "kafka-rdk")) {
    ret.reset(new kafka::rdk::MessagePSKafkaProducer());
    producer_drv_m["kafka-rdk"] = ret;
  }
#endif
#ifdef KAFKA_ASIO_ENABLE
  if ((drvname == "KAFKA-ASIO") || (drvname == "kafka-asio")) {
    ret.reset(new kafka::asio::MessagePSKafkaAsioProducer());
    producer_drv_m["kafka-asio"] = ret;
  }
#endif
  if (ret.get() == NULL) {
    throw chaos::CException(-5, "cannot find a producer driver for:" + drvname, __PRETTY_FUNCTION__);
  }
  MRDDBG_ << drvname << "] created producer:" << std::hex << ret.get();

  if (GlobalConfiguration::getInstance()->hasOption(InitOption::OPT_MSG_PRODUCER_KVP)) {
    std::vector<std::string>           opt = GlobalConfiguration::getInstance()->getOption<std::vector<std::string> >(InitOption::OPT_MSG_PRODUCER_KVP);
    std::map<std::string, std::string> kv;
    fillKVParameter(kv, opt, "");
    for (std::map<std::string, std::string>::iterator i = kv.begin(); i != kv.end(); i++) {
      ret->setOption(i->first, i->second);
    }
  }
  return ret;
}

producer_uptr_t MessagePSDriver::getProducerDriver(const std::string& drvname, const std::string& k) {
  producer_uptr_t ret;

  ChaosLockGuard                        ll(io);
  std::map<std::string, producer_uptr_t>::iterator i = producer_drv_m.find(drvname);
  if (i != producer_drv_m.end()) {
    MRDDBG_ << drvname << "] returning allocated producer:" << std::hex << i->second.get();
    return i->second;
  }
  ret                     = getNewProducerDriver(drvname, k);
  producer_drv_m[drvname] = ret;
  return ret;
}
consumer_uptr_t MessagePSDriver::getNewConsumerDriver(const std::string& drvname, const std::string& gid, const std::string& k) {
  consumer_uptr_t ret;

#ifdef KAFKA_RDK_ENABLE

  if ((drvname == "KAFKA-RDK") || (drvname == "kafka-rdk")) {
    ret.reset(new kafka::rdk::MessagePSRDKafkaConsumer(gid, k));
  }
#endif
#ifdef KAFKA_ASIO_ENABLE
  if ((drvname == "KAFKA-ASIO") || (drvname == "kafka-asio")) {
    ret.reset(new kafka::asio::MessagePSKafkaAsioConsumer(gid, k));
  }
#endif
  if (ret.get() == NULL) {
    throw chaos::CException(-5, "cannot find a consumer driver for:" + drvname, __PRETTY_FUNCTION__);
  }
  MRDDBG_ << drvname << "] created consumer:" << std::hex << ret.get();

  if (GlobalConfiguration::getInstance()->hasOption(InitOption::OPT_MSG_CONSUMER_KVP)) {
    std::vector<std::string>           opt = GlobalConfiguration::getInstance()->getOption<std::vector<std::string> >(InitOption::OPT_MSG_CONSUMER_KVP);
    std::map<std::string, std::string> kv;
    fillKVParameter(kv, opt, "");
    for (std::map<std::string, std::string>::iterator i = kv.begin(); i != kv.end(); i++) {
      ret->setOption(i->first, i->second);
    }
  }
  return ret;
}

consumer_uptr_t MessagePSDriver::getConsumerDriver(const std::string& drvname, const std::string& gid, const std::string& k) {
  std::map<std::string, consumer_uptr_t>::iterator i = consumer_drv_m.find(drvname);

  consumer_uptr_t ret;
  if (i != consumer_drv_m.end()) {
    MRDDBG_ << drvname << "] returning allocated consumer:" << std::hex << i->second.get();

    return i->second;
  }
  ret                     = MessagePSDriver::getNewConsumerDriver(drvname, gid, k);
  consumer_drv_m[drvname] = ret;
  return ret;
}

}  // namespace message
}  // namespace common
}  // namespace chaos
