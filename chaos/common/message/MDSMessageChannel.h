/*
 *	MDSMessageChannel.h
 *	!CHAOS
 *	Created by Bisegni Claudio.
 *
 *    	Copyright 2012 INFN, National Institute of Nuclear Physics
 *
 *    	Licensed under the Apache License, Version 2.0 (the "License");
 *    	you may not use this file except in compliance with the License.
 *    	You may obtain a copy of the License at
 *
 *    	http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */

#ifndef CHAOSFramework_MDSMessageChannel_h
#define CHAOSFramework_MDSMessageChannel_h

#include <chaos/common/message/MultiAddressMessageChannel.h>

#include <set>
#include <vector>

namespace chaos {
    namespace common {
        namespace message {
            
            using namespace std;
            using namespace chaos::common::data;
            //! Message Channel specialize for metadataserver comunication
            /*!
             This class represent a message chanel for comunication with the Metadata Server
             */
            class MDSMessageChannel:
            protected MultiAddressMessageChannel {
                friend class chaos::common::network::NetworkBroker;
            protected:
                //! base constructor
                /*!
                 The base constructor prepare the base class constructor call to be adapted for metadataserver comunication. For the MDS the node address is
                 "system"(ip:port:system)
                 */
                MDSMessageChannel(NetworkBroker *network_broker,
                                  const std::vector<CNetworkAddress>& mds_node_address,
                                  MessageRequestDomainSHRDPtr _new_message_request_domain = MessageRequestDomainSHRDPtr(new MessageRequestDomain()));
                
            public:
                //! return last sendxxx error code
                int32_t getLastErrorCode();
                
                //! return last sendxxx error message
                const std::string& getLastErrorMessage();
                
                //! return last sendxxx error domain
                const std::string& getLastErrorDomain();
                //! Send heartbeat
                /*!
                 Send the heartbeat for an identification ID. This can be an id for a device or an uitoolkit instance.
                 The method return has fast as possible, no aswer is wait
                 \param identificationID identification id of a device or a client
                 */
                void sendHeartBeatForDeviceID(const std::string& identification_id);
                
                //! Send Unit server registration to MDS
                /*!
                 Perform the registration of the unit server
                 \param node_description the description of the unit server to publish
                 \param requestCheck flasg the message has request if it is true
                 \param millisec_to_wait delay after wich the wait is interrupt
                 */
                int sendNodeRegistration(CDataWrapper& node_description,
                                         bool requestCheck = false,
                                         uint32_t millisec_to_wait = 5000);
                
                //! Send node load completion to MDS
                /*!
                 Inform mds that node load phase has completed
                 \param node_information infromation for completion phase
                 \param requestCheck flasg the message has request if it is true
                 \param millisec_to_wait delay after wich the wait is interrupt
                 */
                int sendNodeLoadCompletion(CDataWrapper& node_information,
                                           bool requestCheck = false,
                                           uint32_t millisec_to_wait = 5000);
                
                //! Send Unit server CU states to MDS
                /*!
                 Perform the registration of the unit server
                 \param unitServerDescription the description of the unit server to publish
                 \param requestCheck flasg the message has request if it is true
                 \param millisec_to_wait delay after wich the wait is interrupt
                 */
                
                int sendUnitServerCUStates(CDataWrapper& deviceDataset,
                                           bool requestCheck = false,
                                           uint32_t millisec_to_wait=5000);
                
                //!sendNode heartbeat
                int sentNodeHealthStatus(CDataWrapper& node_health_data,
                                         bool request_check = false,
                                         uint32_t millisec_to_wait=5000);
                
                //! Get all active device id
                /*!
                 Return a list of all device id that are active
                 \param deviceIDVec an array to contain the returned device id
                 \param millisec_to_wait delay after wich the wait is interrupt
                 */
                int getAllDeviceID(vector<string>& deviceIDVec,
                                   uint32_t millisec_to_wait=5000);
                
                //! Get node address for identification id
                /*!
                 Return the node address for an identification id
                 \param identificationID id for wich we need to get the network address information
                 \param deviceNetworkAddress the hadnle to the pointer representing the node address structure to be filled with identification id network info
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int getNetworkAddressForDevice(const std::string& identificationID,
                                               CDeviceNetworkAddress** deviceNetworkAddress,
                                               uint32_t millisec_to_wait=5000);
                
                //! Get curent dataset for device
                /*!
                 Return the node address for an identification id
                 \param identificationID id for wich we need to get the network address information
                 \param deviceDefinition this is the handle to the pointer representig the dataset desprition is returned
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int getLastDatasetForDevice(const std::string& identificationID,
                                            CDataWrapper** deviceDefinition,
                                            uint32_t millisec_to_wait=5000);
                
                //! return the configuration for the data driver
                /*!
                 Return the besta available data service at the monent within the configuraiton for data driver
                 */
                int getDataDriverBestConfiguration(CDataWrapper** deviceDefinition, uint32_t millisec_to_wait=5000);
                
                //!Create a new snapshot
                /*!
                 Start snapshot creation into the mds service
                 \param snapshot_name snapshot name
                 \param node_list node to includ ewithin snapshot
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int createNewSnapshot(const std::string& snapshot_name,
                                      const ChaosStringVector& node_list,
                                      uint32_t millisec_to_wait=5000);
                
                //!Restore a snapshot
                /*!
                 Start the restore operation of snapshot tag. MDS
                 will fire restore action to all node included into the snapshot, then
                 every control unit will perform restore by itself
                 \param snapshot_name snapshot name
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int restoreSnapshot(const std::string& snapshot_name,
                                    uint32_t millisec_to_wait=5000);
                
                //! Delete a snapshoot
                /*!
                 \param snapshot_name snapshot name
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int deleteSnapshot(const std::string& snapshot_name,
                                   uint32_t millisec_to_wait=5000);
                
                //! Search on snapshot
                /*!
                 \param query_filter filter the node that need to be returned
                 \param snapshot_found contains the found snapshot
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int searchSnapshot(const std::string& query_filter,
                                   ChaosStringVector& snapshot_found,
                                   uint32_t millisec_to_wait=5000);
                
                
                //! Search on snapshot
                /*!
                 \param query_filter filter the node that need to be returned
                 \param snapshot_found contains the found snapshot
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int searchSnapshot(const std::string& query_filter,
                                   std::map<uint64_t,std::string>& snapshot_found,uint32_t millisec_to_wait=5000);
                
                //! Search node for snapshot
                /*!
                 \param snapshot_name tha name of the snapshot
                 \param node_found contains the node that are included into the snapshot
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int searchNodeForSnapshot(const std::string& snapshot_name,
                                          ChaosStringVector& node_found,
                                          uint32_t millisec_to_wait=5000);
                
                //! Search snapshot for node
                /*!
                 \param node_uid tha unique id of the node
                 \param snapshot_found contains the snapshot that include the node
                 \param millisec_to_wait delay after wich the wait is interrupt
                 \return error code
                 */
                int searchSnapshotForNode(const std::string& node_uid,
                                          ChaosStringVector& snapshot_found,
                                          uint32_t millisec_to_wait=5000);
                
                //!search for chaos node
                int searchNode(const std::string& unique_id_filter,
                               unsigned int node_type_filter,
                               bool alive_only,
                               unsigned int last_node_sequence_id,
                               unsigned int page_length,
                               ChaosStringVector& node_found,
                               uint32_t millisec_to_wait=5000);
                
                //! send custom message to the servers
                /*!
                 \param message_pack memory is not managed to the ownershiw is not keeped
                 */
                void sendMessage(const std::string& action_domain,
                                 const std::string& action_name,
                                 chaos::common::data::CDataWrapper *message_pack);
                
                //! call an rpc mds function without data
                void callMethod(const std::string& action_domain,
                                const std::string& action_name);
                
                //! send a custom request to the servers
                /*!
                 \param request_pack is the data to forward to the rpc request, memory is internally managed by roc subsystem
                 */
                std::auto_ptr<MultiAddressMessageRequestFuture> sendRequestWithFuture(const std::string& action_domain,
                                                                                      const std::string& action_name,
                                                                                      chaos::common::data::CDataWrapper *request_pack,
                                                                                      int32_t request_timeout = 1000);
                
            };
        }
    }
}
#endif
