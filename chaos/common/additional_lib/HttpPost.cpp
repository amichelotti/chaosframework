#include "HttpPost.h"
#include <chaos/common/global.h>
#include <chaos/common/utility/TimingUtil.h>
#include "mongoose.h"
#define DBG LDBG_ << "[" << __PRETTY_FUNCTION__ << "] "
#define ERR LERR_ << "## [" << __PRETTY_FUNCTION__ << "] "

namespace chaos {
namespace common {
namespace http {
static int s_exit_flag = 0, counter = 0;
//     static std::stringstream bufres;

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *)ev_data;
  int                  connect_status;

  switch (ev) {
    case MG_EV_CONNECT:
      connect_status = *(int *)ev_data;
      if (connect_status != 0) {
          s_exit_flag = -1;
          DBG << "connect status:" << connect_status<<" error:"<<strerror(connect_status)<<" exit:"<<s_exit_flag;

      }
      break;
    case MG_EV_HTTP_REPLY: {
      std::stringstream *ptr = (std::stringstream *)nc->mgr->user_data;
      //  LDBG_<<"Got reply:"<< hm->body.len<<" resp:"<< hm->body.p,hm->resp_code;
    //  DBG<<"REPLY hm:"<<hm<<" len:"<<hm->body.len;
      if (ptr && hm && hm->body.p && hm->body.len) {
        ptr->write((char *)hm->body.p, hm->body.len);
        // LDBG_<<hm->body.len<<"] Read:"<<(char*)hm->body.p;

        // LDBG_<<"Read:"<<ptr->str();
      }
      // printf("Got reply:\n%.*s\n%d\n", (int) hm->body.len, hm->body.p,hm->resp_code);

      nc->flags |= MG_F_SEND_AND_CLOSE;
      s_exit_flag = hm->resp_code;
      break;
    }
    case MG_EV_CLOSE:
     // DBG<<"CLOSE:"<<s_exit_flag;
      if (s_exit_flag == 0) {
        s_exit_flag = 1;

        //    printf("Server closed connection\n");
      };
      break;
    default:
    //  ERR << "MONGOOSE DEFAULT:" << s_exit_flag << " EV:" << ev << " counter:" << counter;
      break;
  }
}
HttpPost::HttpPost(const std::string &id, uint32_t timeout_ms, uint32_t _retry_offline_ms)
    : clientid(id), mgr(NULL), timeo(timeout_ms), retry_offline_ms(_retry_offline_ms) {
  mgr = (void *)calloc(1, sizeof(struct mg_mgr));
}
HttpPost::~HttpPost() {
  free(mgr);
}
static ChaosMutex devio_mutex;

int HttpPost::post(const std::string &server, const std::string &api, const std::string &body, std::stringstream &res) {
  char s_url[256];
  int  ret;
 // DBG << "before mutex offline:" << off_line.size();

  ChaosLockGuard l(devio_mutex);
 if (off_line.count(server) && (chaos::common::utility::TimingUtil::getTimeStamp() - off_line[server]) < retry_offline_ms) {
    ERR << "server " << server << " has put offline for " << (retry_offline_ms - (chaos::common::utility::TimingUtil::getTimeStamp() - off_line[server])) << " ms";

    return 503;  // SERVICE UNAVAILABLE
  }
 
  mg_mgr_init((struct mg_mgr *)mgr, (void *)&res);

  snprintf(s_url, sizeof(s_url), "%s/%s", server.c_str(), api.c_str());
  // struct mg_connection *nc;
  struct mg_mgr *p = (struct mg_mgr *)mgr;
  s_exit_flag      = 0;
  counter          = 0;
  // LDBG_<<"Connecting to:"<<s_url<<" body:"<<body;
 // DBG << "connecting " << s_url;

  mg_connect_http(p, ev_handler, s_url, "Content-Type:application/json\r\n", body.c_str());
  while ((s_exit_flag == 0) && (counter < timeo)) {
    mg_mgr_poll(p, 1);
    counter++;
  }

  //DBG << "exit polling " << s_exit_flag << " counter:" << counter;

  if (counter == timeo) {
    off_line[server] = chaos::common::utility::TimingUtil::getTimeStamp();
    ERR << server << " timeout of:" << timeo << " at:" << off_line[server] << " put offline for:" << retry_offline_ms << " ms";

    return 504;  // REQUEST GATEWAY TIMEOUT
  }

  // LDBG_<<"exit:"<<s_exit_flag<<" body:"<<res.str();
  if (s_exit_flag == 200) {
    if (off_line.erase(server)) {
      DBG << "server " << server << " is online";
    }
  } else if (s_exit_flag == -1) {
    off_line[server] = chaos::common::utility::TimingUtil::getTimeStamp();
    ERR << "server " << server << " connection error at:" << off_line[server] << " put offline for:" << retry_offline_ms << " ms";

    return 502;
  }
  return s_exit_flag;
}

}  // namespace http
}  // namespace common
}  // namespace chaos
