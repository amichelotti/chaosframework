/* 
 * File:   ChaosManager.h
 * Author: Andrea Michelotti
 *
 * Created on S21/4/2021
 */

#ifndef __ChaosManager_H
#define __ChaosManager_H

//#include <chaos/common/chaos_constants.h>

#include <chaos/common/utility/SingletonCW.h>
#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/utility/InizializableService.h>
#include <chaos/common/io/IODataDriver.h>

#define DEFAULT_TIMEOUT_FOR_CONTROLLER 10000000
namespace chaos {
namespace common {
namespace cache_system {
class CacheDriver;
}
}  // namespace common
namespace service_common {
namespace persistence {
namespace data_access {
class AbstractPersistenceDriver;
}
}  // namespace persistence

class ChaosManager : public chaos::common::utility::SingletonCW<ChaosManager>{
        friend class chaos::common::utility::SingletonCW<ChaosManager>;

 private:
  chaos::common::cache_system::CacheDriver*                                   cache_driver;
  chaos::service_common::persistence::data_access::AbstractPersistenceDriver* persistence_driver;

  //  ::common::misc::data::DBbase* db;
  // NetworkBroker *broker;
  //chaos::common::message::MDSMessageChannel *mdsChannel;
  //! Device MEssage channel to control via chaos rpc the device
  //chaos::common::message::DeviceMessageChannel *deviceChannel;
  //! The io driver for accessing live data of the device

  ChaosManager(const chaos::common::data::CDataWrapper&cw);
   virtual ~ChaosManager();
  int init(const chaos::common::data::CDataWrapper& conf);
  uint64_t tot_us;
  uint32_t naccess;
 public:

  bool hasDirectCache() const {return (cache_driver!=NULL);}

  bool hasDirectPersistence() const {return (persistence_driver!=NULL);}
  chaos::common::io::IODataDriverShrdPtr live_driver;

  void init(void*);
  chaos::common::data::CDWShrdPtr  getLiveChannel(const std::string& key, int domain);
  chaos::common::data::VectorCDWShrdPtr getLiveChannel(const std::vector<std::string>& channels);

  int nodeSearch(const std::string&              unique_id_filter,
                 chaos::NodeType::NodeSearchType node_type_filter,
                 bool                            alive_only,
                 unsigned int                    last_node_sequence_id,
                 unsigned int                    page_length,
                 uint64_t&                       lastid,
                 ChaosStringVector&              node_found,
                 uint32_t                        millisec_to_wait=5000,
                 const std::string&              impl="");
 chaos::common::data::CDWUniquePtr nodeGetDescription(const std::string& uid);
chaos::common::data::CDWUniquePtr cuGetFullDescription(const std::string& uid);
chaos::common::data::CDWUniquePtr getVariable(const std::string& uid);
chaos::common::data::CDWUniquePtr setVariable(const std::string& uid,const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr removeVariable(const std::string& uid);


};
}  // namespace service_common
}  // namespace chaos
#endif /* ChaosManager_H */
