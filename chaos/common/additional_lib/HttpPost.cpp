#include "HttpPost.h"
#include "mongoose.h"
#include <chaos/common/global.h>


namespace chaos{
    namespace common{
        namespace http{
        static int s_exit_flag = 0;

            static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
  struct http_message *hm = (struct http_message *)ev_data;
  int connect_status;

  switch (ev)
  {
  case MG_EV_CONNECT:
    connect_status = *(int *)ev_data;
    if (connect_status != 0)
    {
      // printf("Error connecting to %s: %s\n", hm->uri, strerror(connect_status));
      s_exit_flag = -1;
    }
    break;
  case MG_EV_HTTP_REPLY:{
    std::stringstream *ptr= (std::stringstream *)nc->mgr->user_data;

    //DPRINT("Got reply:\n%.*s\n%d\n", (int) hm->body.len, hm->body.p,hm->resp_code);
    if(ptr&&hm->body.p && hm->body.len){
        ptr->write((char*)hm->body.p,hm->body.len);
        //LDBG_<<hm->body.len<<"] Read:"<<(char*)hm->body.p;

       // LDBG_<<"Read:"<<ptr->str();
    }
    // printf("Got reply:\n%.*s\n%d\n", (int) hm->body.len, hm->body.p,hm->resp_code);
    
    nc->flags |= MG_F_SEND_AND_CLOSE;
    s_exit_flag = hm->resp_code;
    break;
  }
  case MG_EV_CLOSE:
    if (s_exit_flag == 0)
    {
      //    printf("Server closed connection\n");
  //    s_exit_flag = 1;
    };
    break;
  default:
    break;
  }
}
            HttpPost::HttpPost(const std::string& id):clientid(id),mgr(NULL){
                mgr=(void*)malloc(sizeof(struct mg_mgr));
                mg_mgr_init((struct mg_mgr*)mgr,NULL);
            }   
            HttpPost::~HttpPost(){
                free(mgr);
            }
   
            int HttpPost::post(const std::string& server,const std::string&api,const std::string& body,std::stringstream& res){
                char s_url[256];
                int ret;
                snprintf(s_url, sizeof(s_url), "%s/%s", server.c_str(), api.c_str());
                s_exit_flag = 0;
                struct mg_connection *nc;
                struct mg_mgr* p=(struct mg_mgr*)mgr;
                p->user_data=(void*)&res;
                nc = mg_connect_http(p, ev_handler, s_url, "Content-Type:application/json\r\n", body.c_str());
                while (s_exit_flag == 0){
                    mg_mgr_poll((struct mg_mgr*)mgr, 1);
                }
  
                return s_exit_flag;
            }


            
        }
    }
}
