#include "MessagePSKafkaProducer.h"
#include <chaos/common/global.h>
#include <librdkafka/rdkafka.h>
#include <signal.h>
#define MRDAPP_ INFO_LOG(MessagePSKafkaProducer)
#define MRDDBG_ DBG_LOG(MessagePSKafkaProducer)
#define MRDERR_ ERR_LOG(MessagePSKafkaProducer)

namespace chaos {
namespace common {
namespace message {
namespace kafka {
namespace rdk {

static void dr_msg_cb(rd_kafka_t*               rk,
                      const rd_kafka_message_t* rkmessage,
                      void*                     opaque) {
    
     MessagePSKafkaProducer* mp=(MessagePSKafkaProducer*)rkmessage->_private;
     if(mp){
      mp->HandleRequest(rk,rkmessage);
     }                   
}
void MessagePSKafkaProducer::HandleRequest(rd_kafka_t*               rk,
                      const rd_kafka_message_t* rkmessage){
/**/
  if(msg_opt==MSG_NOCOPY){
    if(todestroy.count(rkmessage->payload)){
  //MRDDBG_<<"Erasing Payload "<<std::hex<<(void*)rkmessage->payload;

      todestroy.erase(rkmessage->payload);
    } else {
        MRDERR_<<" Payload "<<std::hex<<(void*)rkmessage->payload<<" not found, in queue:"<<todestroy.size();
    }
  }
  if (rkmessage->err){

    stats.last_err=1;
    stats.errs++;
    MRDERR_ << "["<<stats.counter<<","<<stats.oks<<","<<stats.errs<<"]" << " Message delivery failed:" << rd_kafka_err2str(rkmessage->err);
    MRDERR_<<" key:"<<rkmessage->key<<" len:"<<rkmessage->len<<" off:"<<rkmessage->offset<< " part:"<<rkmessage->partition;

    return;
  }
  stats.counter++;
  stats.last_err=0;
  stats.oks++;
  if (handlers.size()) {
  }


    //fprintf(stderr, "%% Message delivery failed: %s\n", rd_kafka_err2str(rkmessage->err));
  /* The rkmessage is destroyed automatically by librdkafka */
}
MessagePSKafkaProducer::~MessagePSKafkaProducer() {
    MRDDBG_<<" DESTROY PRODUCER";
  todestroy.clear();
  MessagePublishSubscribeBase::stop();
  flush(60*1000);

  /* If the output queue is still not empty there is an issue
         * with producing messages to the clusters. */
    rd_kafka_abort_transaction(rk,60*1000);

  /* Destroy the producer instance */
  /*char tmp[16];
   snprintf(tmp, sizeof(tmp), "%i", SIGIO);  
   setOption("internal.termination.signal",tmp);
*/

}
int MessagePSKafkaProducer::flush(const int timeo){
  //MRDDBG_ << "Flushing... ";
  ChaosLockGuard ll(io);

    rd_kafka_flush(rk, timeo);
if (rd_kafka_outq_len(rk) > 0){
    std::stringstream ss;
    ss<<rd_kafka_outq_len(rk)<<" messages were not delivered";
    errstr=ss.str();

    return -1;
}
 // MRDDBG_ << "Flushing...done ";
  todestroy.clear();

return 0;

}

MessagePSKafkaProducer::MessagePSKafkaProducer():chaos::common::message::MessagePublishSubscribeBase("kafka-rdk") {
  running = false;
}
MessagePSKafkaProducer::MessagePSKafkaProducer(const std::string& k):chaos::common::message::MessagePublishSubscribeBase("kafka-rdk") {
}
int MessagePSKafkaProducer::pushMsg(const chaos::common::data::CDataWrapper& data, const std::string& key,const int32_t pnum) {
  return 0;
}
  int MessagePSKafkaProducer::deleteKey(const std::string&key){
    return 0;
  }

int MessagePSKafkaProducer::setOption(const std::string& key, const std::string& value){
    MRDDBG_<<"Setting Option:"<<key<<"="<<value;

  return MessagePSRDKafka::setOption(key,value);
  }

int MessagePSKafkaProducer::applyConfiguration() {
  int  ret = 0;
  char ers[512];
          
  MRDDBG_ << "Apply configuration";
        

  ChaosLockGuard ll(io);
  
  if ((ret = MessagePSRDKafka::init(servers)) == 0) {
    if(rk==NULL){
      MRDDBG_ << "create new producer";
      setMaxMsgSize(16*1024*1024);
      rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

      if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, ers, sizeof(ers)))) {
        errstr=ers;
        MRDERR_ << "Failed to create new producer: " << errstr;
        return -10;
      }
     // if (handlers.size()) {
      //}
    }
    
  }
  return ret;
}


int MessagePSKafkaProducer::pushMsgAsync(const chaos::common::data::CDataWrapper& data, const std::string& key,const int32_t pnum) {
  rd_kafka_resp_err_t err;
  int32_t                size;
  std::string         topic = key;
  std::replace(topic.begin(), topic.end(), '/', '.');
  std::replace(topic.begin(), topic.end(), ':', '.');

  if(rk==NULL){
      MRDERR_ << "Not applied configuration" << errstr;
      errstr="Not applied configuration";
      return -11;
  }
 // ChaosLockGuard ll(io);

//MRDDBG_ << "pushing: " << size<<" d:"<<data.getJSONString();
retry:
//int32_t siz;
//  int32_t *bslen=(int32_t *);
  //MRDDBG_ <<"NOTIFY:("<<data.getBSONRawSize()<<","<<*bslen<<","<<siz<<"):"<<data.getJSONString();
    void* ptr=(void*)data.getBSONRawData(size);

  err = rd_kafka_producev(
      /* Producer handle */
      rk,
      /* Topic name */
      RD_KAFKA_V_TOPIC(topic.c_str()),
      /* Make a copy of the payload. */
    //  RD_KAFKA_V_MSGFLAGS(((synchronous)?0:RD_KAFKA_MSG_F_COPY)),
      RD_KAFKA_V_MSGFLAGS(((msg_opt==chaos::common::message::MessagePublishSubscribeBase::MSG_COPY)?RD_KAFKA_MSG_F_COPY:0)), // let the caller deallocate
      /* Message value and length */
      RD_KAFKA_V_VALUE(ptr, size),
      /* Per-Message opaque, provided in
                         * delivery report callback as
                         * msg_opaque. */
      RD_KAFKA_V_OPAQUE(this),
      /* End sentinel */
      RD_KAFKA_V_END);


  if (err) {
    /*
                         * Failed to *enqueue* message for producing.
                         */

    MRDERR_ << "["<<stats.counter<<","<<stats.oks<<","<<stats.errs<<"] Failed to produce to topic '" << topic <<"' err:"<< rd_kafka_err2str(err)<<" size:"<<size;

    if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
      /* If the internal queue is full, wait for
                                 * messages to be delivered and then retry.
                                 * The internal queue represents both
                                 * messages to be sent and messages that have
                                 * been sent or failed, awaiting their
                                 * delivery report callback to be called.
                                 *
                                 * The internal queue is limited by the
                                 * configuration property
                                 * queue.buffering.max.messages */
      rd_kafka_poll(rk, 100 /*block for max 1000ms*/);

      goto retry;
    }
  }
  if(msg_opt==chaos::common::message::MessagePublishSubscribeBase::MSG_SYNCH){
    err= rd_kafka_flush	(rk,1000);
  } else {
    if((err==0)&&(msg_opt==chaos::common::message::MessagePublishSubscribeBase::MSG_NOCOPY)){
      todestroy[ptr]=data.getBSONShrPtr();
    }

    rd_kafka_poll(rk, 0 /*non-blocking*/);
  }
    stats.counter++;

  if(err==0){
    stats.oks++;
    return stats.last_err;
  } else {
    stats.errs++;

  }
  return err;
}
void MessagePSKafkaProducer::poll() {
      rd_kafka_poll(rk, 500 /*block for max 1000ms*/);
}
}  // namespace rdk
}  // namespace kafka
}  // namespace message
}  // namespace common
}  // namespace chaos