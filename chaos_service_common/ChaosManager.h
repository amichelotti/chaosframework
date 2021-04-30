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
#include <chaos/common/batch_command/BatchCommandTypes.h>
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
  ChaosManager();

  ChaosManager(const chaos::common::data::CDataWrapper&cw);
   virtual ~ChaosManager();
  int init(const chaos::common::data::CDataWrapper& conf);
  uint64_t tot_us;
  uint32_t naccess;
 public:

  bool hasDirectCache() const {return (cache_driver!=NULL);}

  bool hasDirectPersistence() const {return (persistence_driver!=NULL);}
  chaos::common::io::IODataDriverShrdPtr live_driver;

    chaos::common::data::CDWShrdPtr  getLiveChannel(const std::string& key);

  chaos::common::data::CDWShrdPtr  getLiveChannel(const std::string& key, int domain);
  chaos::common::data::VectorCDWShrdPtr getLiveChannel(const std::vector<std::string>& channels);

  int nodeSearch(const std::string&              unique_id_filter,
                 chaos::NodeType::NodeSearchType node_type_filter,
                 bool                            alive_only,
                 unsigned int                    last_node_sequence_id,
                 unsigned int                    page_length,
                 uint32_t&                       numberofpage,
                 ChaosStringVector&              node_found,
                 uint32_t                        millisec_to_wait=5000,
                 const std::string&              impl="");
 chaos::common::data::CDWUniquePtr nodeGetDescription(const std::string& uid);
chaos::common::data::CDWUniquePtr cuGetFullDescription(const std::string& uid);
chaos::common::data::CDWUniquePtr getVariable(const std::string& uid);
chaos::common::data::CDWUniquePtr setVariable(const std::string& uid,const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr removeVariable(const std::string& uid);
chaos::common::data::CDWUniquePtr nodeDelete(const std::string& uid,const std::string parent="");
chaos::common::data::CDWUniquePtr nodeNew(const std::string& uid,const chaos::common::data::CDataWrapper& value,const std::string parent="");
chaos::common::data::CDWUniquePtr getFullUnitServer(const std::string& uid);
chaos::common::data::CDWUniquePtr setFullUnitServer(const std::string& uid,const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr getAgentForNode(const std::string& uid);
chaos::common::data::CDWUniquePtr updateProperty(const std::string& uid,const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr manageCUType(const std::string& uid,const std::string& implname,int op=0);
chaos::common::data::CDWUniquePtr setInstanceDescription(const std::string& uid,const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr deleteInstance(const std::string& uid,const std::string&parent);
chaos::common::data::CDWUniquePtr getCUInstance(const std::string& uid);
chaos::common::data::CDWUniquePtr startStop(const std::string& uid,bool start);
chaos::common::data::CDWUniquePtr initDeinit(const std::string& uid,bool ini);
chaos::common::data::CDWUniquePtr loadUnloadControlUnit(const std::string& uid,bool ini);
chaos::common::data::CDWUniquePtr searchLogEntry(const std::string& uid,const std::vector<std::string>& domains,uint64_t start,uint64_t end,uint64_t seq,uint32_t page);
chaos::common::data::CDWUniquePtr searchScript(const std::string& uid,uint64_t start,uint32_t page);

chaos::common::data::CDWUniquePtr saveScript(const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr removeScript(const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr manageScriptInstance(const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr loadFullScript(const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr loadFullDescription(const std::string&);

chaos::common::data::CDWUniquePtr setNodeDescription(const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr setInputDatasetAttributeValues(const std::string&uid, const std::string&key,const std::string&value);
chaos::common::data::CDWUniquePtr setInputDatasetAttributeValues(const std::string&uid,  std::map<const std::string,const std::string>& keyvalue);
chaos::common::data::CDWUniquePtr commandTemplateSubmit(const std::string&uid,const std::string& command_alias,const chaos::common::data::CDWUniquePtr& slow_command_data,const chaos::common::batch_command::SubmissionRuleType::SubmissionRule submission_rule,const uint32_t priority,const uint64_t scheduler_steps_delay,const uint32_t submission_checker_steps_delay);
ChaosStringVector getSnapshotForNode(const std::string&uid);
chaos::common::data::CDWUniquePtr createNewSnapshot(const std::string& snapshot_name,const ChaosStringVector& node_list); 
chaos::common::data::CDWUniquePtr restoreSnapshot(const std::string& snapshot_name);
std::map<uint64_t,std::string> getAllSnapshot(const std::string& query_filter);
ChaosStringVector getNodesForSnapshot(const std::string& query_filter);
std::map<uint64_t, std::string> getAllSnapshotOfCU(const std::string& cu);
chaos::common::data::CDWUniquePtr deleteSnapshot(const std::string& cu);
chaos::common::data::CDWUniquePtr getSnapshotDatasetForNode(const std::string& snapname, const std::string& node_uid);

chaos::common::data::CDWUniquePtr agentNodeOperation(const std::string& node,int32_t op);
chaos::common::data::CDWUniquePtr saveNodeAssociation(const std::string&name,const chaos::common::data::CDataWrapper& value);
chaos::common::data::CDWUniquePtr loadNodeAssociation(const std::string&name,const std::string&parent);
chaos::common::data::CDWUniquePtr listNodeForAgent(const std::string&name);
chaos::common::data::CDWUniquePtr loadAgentDescription(const std::string&name,bool loaddata=true);
chaos::common::data::CDWUniquePtr checkAgentHostedProcess(const std::string&name);

chaos::common::data::CDWUniquePtr clearCommandQueue(const std::string&name);
chaos::common::data::CDWUniquePtr killCurrentCommand(const std::string&name);

};
}  // namespace service_common
}  // namespace chaos
#endif /* ChaosManager_H */
