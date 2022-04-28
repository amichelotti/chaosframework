#include <chaos/common/utility/Singleton.h>
#include <chaos/common/utility/InizializableService.h>
namespace nadjieb {
    class MJPEGStreamer;
}

namespace chaos {
	namespace common {
		namespace direct_io {
            class HttpStreamManager:
            public chaos::common::utility::Singleton<HttpStreamManager>,
            public chaos::common::utility::InizializableService {
        
            nadjieb::MJPEGStreamer* streamer;
            std::string rootpath;
            public:
            HttpStreamManager();
            ~HttpStreamManager();
            std::string getRoot() const {return rootpath;}
            void publish(const std::string&path,const std::string&data);
            void init(void *init_data);
            void deinit();
        };
        }
        }
        }