#ifndef __HTTP_POST__
#define __HTTP_POST__
#include <string>
#include <sstream>
#include <stdint.h>
#include <map>
namespace chaos{
    namespace common{
        namespace http{
            class HttpPost{
                std::string clientid;
                void *mgr;
                const uint32_t timeo;
                std::map<std::string,uint64_t> off_line;
                const uint32_t retry_offline_ms;
                public:
                HttpPost(const std::string& id="Chaos HttpPost",uint32_t timeout_ms=500,uint32_t _retry_offline_ms=10000);
                ~HttpPost();

                /**
                 * @brief Make a post request
                 * 
                 * @param server server:port 
                 * @param api api to call
                 * @param res return body 
                 * @return int return htttp code (200 is ok)
                 */
                int post(const std::string& server,const std::string&api,const std::string& body,std::stringstream& res);


            };
        }
    }
}
#endif