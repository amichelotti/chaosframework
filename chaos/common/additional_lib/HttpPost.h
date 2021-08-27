#ifndef __HTTP_POST__
#define __HTTP_POST__
#include <string>
#include <sstream>
namespace chaos{
    namespace common{
        namespace http{
            class HttpPost{
                std::string clientid;
                void *mgr;
                public:
                HttpPost(const std::string& id="Chaos HttpPost");
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