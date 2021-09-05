#ifndef __HTTP_POST__
#define __HTTP_POST__
#include <string>
#include <sstream>
#include <stdint.h>
namespace chaos{
    namespace common{
        namespace http{
            class HttpPost{
                std::string clientid;
                void *mgr;
                const uint32_t timeo;
                int counter;
                public:
                HttpPost(const std::string& id="Chaos HttpPost",uint32_t timeout_ms=2000);
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