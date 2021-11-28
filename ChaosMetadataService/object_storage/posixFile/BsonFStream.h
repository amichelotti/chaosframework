#include <chaos/common/data/CDataWrapper.h>
#include <boost/iostreams/device/mapped_file.hpp>


class BsonFStream{

    bson_writer_t *writer;
    std::string name;
    uint32_t objs;
    uint8_t*       buf;
    size_t         buflen,fsize;
    uint64_t       open_ts,close_ts;
    public:
    BsonFStream(const std::string&fname,int inital_size=1024*1024);
    BsonFStream();
    boost::iostreams::mapped_file mf;
    ChaosMutex wmutex;

    ~BsonFStream();
    int open(const std::string&fname,int initial_size=1024*1024);
    size_t size();
    uint32_t nobj();
    int close();
    int write(const std::string&key,const chaos::common::data::CDataWrapper&);
    int write(const std::string&key,const bson_value_t*);
    std::string getName()const {return name;}

};