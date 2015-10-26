/*
 *	chaos_constants.h
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
#ifndef ChaosFramework_ConstrolSystemConstants_h
#define ChaosFramework_ConstrolSystemConstants_h

#include <chaos/common/batch_command/BatchCommandConstants.h>


namespace chaos {
    
    /** @defgroup CommonParamOption Common Layer Configuration Option
     *  This is the collection of the parameter for common layer to customize the initial setup
     *  @{
     */
    //! groups the default !CHAOS option keys used in command line or config file
    namespace InitOption{
        //! for print the help
        static const char * const	OPT_HELP                            = "help";
        //! config file parameter
        static const char * const   OPT_CONF_FILE						= "conf-file";
        //! Specify when the log must be forwarded on console
        static const char * const   OPT_LOG_ON_CONSOLE                  = "log-on-console";
        //! Specify when the log must be forwarded on file
        static const char * const   OPT_LOG_ON_FILE                     = "log-on-file";
        //! Specify when the file path of the log
        static const char * const   OPT_LOG_FILE                        = "log-file";
        //! Specify the level of the log going
        static const char * const   OPT_LOG_LEVEL                       = "log-level";
        //! Specify the log max size
        static const char * const   OPT_LOG_MAX_SIZE_MB                 = "log-max-size";
        //! enable metric loggin on console backend
        static const char * const   OPT_LOG_METRIC_ON_CONSOLE           = "log-metric-on-console";
        //! enable metric loggin on file backend
        static const char * const   OPT_LOG_METRIC_ON_FILE              = "log-metric-on-file";
        //! enable metric loggin on file backend
        static const char * const   OPT_LOG_METRIC_ON_FILE_PATH         = "log-metric-on-file-path";
        //! activate the loggin of metric on rpc system
        static const char * const   OPT_RPC_LOG_METRIC                  = "rpc-log-metric";
        //! the time between the update of rpc metric on persistence backend (in seconds)
        static const char * const   OPT_RPC_LOG_METRIC_UPDATE_INTERVAL  = "rpc-log-metric-update-interval";
        //! Specify the implementation to use for rp messaging
        static const char * const   OPT_RPC_IMPL_KV_PARAM               = "rpc-kv-param";
        //! Specify the implementation to use for rp messaging
        static const char * const   OPT_RPC_IMPLEMENTATION              = "rpc-server-impl";
        //! Specify the implementation to use for rp messaging
        static const char * const   OPT_RPC_SYNC_ENABLE                 = "rpc-syncserver-enable";
        //! Specify the implementation to use for rp messaging
        static const char * const   OPT_RPC_SYNC_IMPLEMENTATION         = "rpc-syncserver-impl";
        //! Specify the implementation to use for rp messaging
        static const char * const   OPT_RPC_SYNC_PORT					= "rpc-syncserver-port";
        //! Specify the network port where rpc system will publish al the service
        static const char * const   OPT_RPC_SERVER_PORT                 = "rpc-server-port";
        //! Specify the number of the thread that the rpc ssytem must use to process the request
        static const char * const   OPT_RPC_SERVER_THREAD_NUMBER        = "rpc-server-thread-number";
        //! Specify the implementation to use for the direct io subsystem
        static const char * const   OPT_DIRECT_IO_IMPLEMENTATION		= "direct-io-impl";
        //! Specify the network port where the direct io subsystem publish i's priority channel
        static const char * const   OPT_DIRECT_IO_PRIORITY_SERVER_PORT  = "direct-io-priority-server-port";
        //! Specify the network port where the direct io subsystem publish i's priority channel
        static const char * const   OPT_DIRECT_IO_SERVICE_SERVER_PORT   = "direct-io-service-server-port";
        //! Specify the number of the thread that the direct io subsystem must use to process the request
        static const char * const   OPT_DIRECT_IO_SERVER_THREAD_NUMBER  = "direct-io-server-thread-number";
        //! activate the loggin of metric on rpc system
        static const char * const   OPT_DIRECT_IO_LOG_METRIC            = "direct-io-log-metric";
        //! activate the merged or separate metric on target endpoint
        static const char * const   OPT_DIRECT_IO_CLIENT_LOG_METRIC_MERGED_ENDPOINT = "direct-io-client-log-metric-mep";
        //! the time between the update of rpc metric on persistence backend (in seconds)
        static const char * const   OPT_DIRECT_IO_LOG_METRIC_UPDATE_INTERVAL = "direct-io-log-metric-update-interval";
        //!disable the event system
        static const char * const   OPT_EVENT_DISABLE                   = "event-disable";
        //! Specify the metadata address for the metadataserver
        static const char * const   OPT_METADATASERVER_ADDRESS          = "metadata-server";
        //! Specify the ip where publish the framework
        static const char * const   OPT_PUBLISHING_IP                   = "publishing-ip";
        //! use the interface name to determinate the ip where publish itself
        static const char * const   OPT_PUBLISHING_INTERFACE            = "publishing-interface";
        //! Specify the ip where publish the framework
        static const char * const   OPT_DATA_IO_IMPL					= "data-io-impl";
    }
    /** @} */ // end of ParamOption
    
    /** @defgroup RpcConfigurationKey Rpc System Configuraiton
     *  This is the collection of the key for the parameter used to configura rpc system
     *  @{
     */
    //! Name space for grupping the options used for the rpc system configuration
    namespace RpcConfigurationKey {
        //! the regular expression for check the wel format key/valuparameter list for CS_CMDM_OPT_RPC_IMPL_KV_PARAM
        static const char * const OPT_RPC_IMPL_KV_PARAM_STRING_REGEX    = "([a-zA-Z0-9/_,.]+)=([a-zA-Z0-9/_,.]+)(\\|([a-zA-Z0-9/_,.]+)=([a-zA-Z0-9/_,.]+))*";
    }
    /** @} */ // end of RpcConfigurationKey
    
    namespace common {
        namespace direct_io {
            /** @defgroup DirectIOConfigurationKey DirectIO System Configuration
             *  This is the collection of the key for the parameters used to configure the DirectIO system
             *  @{
             */
            //! Name space for grupping option used for rpc system configuration
            namespace DirectIOConfigurationKey {
                //!  specify the type of the adatpre to be used
                static const char * const DIRECT_IO_IMPL_TYPE						= "direct_io_impl_type";
                //!  the number of the thread to user
                static const char * const DIRECT_IO_SERVER_THREAD_NUMBER			= "direct_io_server_tn";
                //!  specify the port where the rpc must publish the his socket
                static const char * const DIRECT_IO_PRIORITY_PORT					= "direct_io_priority_port";
                //!  specify the port where the rpc must publish the his socket
                static const char * const DIRECT_IO_SERVICE_PORT					= "direct_io_service_port";
                
            }
            /** @} */ // end of DirectIOConfigurationKey
        }
    }
    
    /** @defgroup NodeDefinitionKey !CHAOS node key description
     *  This is the collection of the key for the general node information
     *  @{
     */
    //! Name space for grupping key for the node infromation
    namespace NodeDefinitionKey {
        //! define the node unique identification[string]
        /*!
         Identify in a unique way the node within a domain. A node can be
         every node except the metadata server.
         */
        static const char * const NODE_UNIQUE_ID        = "ndk_uid";
        
        //! defines the unique id that is parent of target node [string]
        /*!
         Identify an unique id that is the parent of the node
         */
        static const char * const NODE_PARENT           = "ndk_parent";
        
        //! defines at which group the node belong [array of unique id]
        /*!
         Every entry of the array is a unique id of a group
         */
        static const char * const NODE_GROUP_SET       = "ndk_group_set";
        
        //! define the node type[string]
        /*!
         The type permit to identify kind of node is. For example
         contorl unit(slow or realtime) ui, eu, etc...
         */
        static const char * const NODE_TYPE             = "ndk_type";
        
        //! define the node type[string]
        /*!
         The subtype related to the type
         */
        static const char * const NODE_SUB_TYPE         = "ndk_sub_type";
        
        //! identify the node rpc address[string:string]
        /*!
         Permit to assciate to the node the address of the rpc interface
         hta is given by the network brocker where the node si attacched.
         */
        static const char * const NODE_RPC_ADDR         = "ndk_rpc_addr";
        
        //! identify the node rpc address[string:string]
        /*!
         si the address direct io base address(whitout endpoint) published by the node
         */
        static const char * const NODE_DIRECT_IO_ADDR   = "ndk_d_io_addr";
        
        //! identify the domain within the rpc infrastructure [string]
        /*!
         Identify the domain where the node has registered his api.
         */
        static const char * const NODE_RPC_DOMAIN       = "ndk_rpc_dom";
        
        //! identify the node security key[string]
        /*!
         the security key is a public and private key standard
         thank permit to a node to be securily identifyed.
         */
        static const char * const NODE_SECURITY_KEY     = "ndk_sec_key";
        
        
        //! is the hartbeat of the node for the current request[uint64]
        static const char * const NODE_TIMESTAMP        = "ndk_heartbeat";
    }
    /** @} */ // end of NodeDefinitionKey
    
    /** @defgroup NodeType !CHAOS node type value description
     *  this is the list of the all default node type recognized automatically by !CHAOS
     * infrastructure
     *  @{
     */
    //! Name space for grupping key for the node type
    namespace NodeType {
        //! identify a single process healt service
        /*!
         Every common toolkit singleton(every process) has an internal healt system
         taht register in an syncrhonous way his presence.
         */
        static const char * const NODE_TYPE_HEALT_PROCESS     = "nt_healt_process";
        
        //! identify an unit server node
        /*!
         A unit server node is a specialized node that act as server for many configurable
         constrol unit.
         */
        static const char * const NODE_TYPE_UNIT_SERVER     = "nt_unit_server";
        //! identify a control unit node
        /*!
         A CU node is a tipical !CHAOS node that act as controller of hardware of
         other chaos node.
         */
        static const char * const NODE_TYPE_CONTROL_UNIT    = "nt_control_unit";
        //! identify an user itnerface node
        /*!
         An user interface is a node that can control other nodes and show
         data from it, tipically a program achieve to monitor the system and
         send command to change the his state.
         */
        static const char * const NODE_TYPE_USER_INTERFACE  = "nt_user_interface";
        //! identify a data service node
        /*!
         A data service node is a !CHAOS service that manage the query on producer data
         */
        static const char * const NODE_TYPE_DATA_SERVICE    = "nt_data_service";
        
        //! identify a wan proxy node
        /*!
         A wan proxy node is a node that permit to adapt the wan syncrhonous worls to
         !CHAOS async one.
         */
        static const char * const NODE_TYPE_WAN_PROXY       = "nt_wan_proxy";
    }
    /** @} */ // end of NodeType
    
    /** @defgroup NodeHealtDefinitionKey !CHAOS node healt key description
     *  @{
     */
    //! This namespace enclose all the key for inspetting a node healt
    //! every different node need to expose default key and custom
    namespace NodeHealtDefinitionKey {
        static const char * const HEALT_KEY_POSTFIX                     = "_healt";
        //! define time stamp of the push (usefull for heart beating)
        static const char * const NODE_HEALT_TIMESTAMP                  = "nh_ts";
        //! define time stamp of the collection of the last insert metric
        static const char * const NODE_HEALT_TIMESTAMP_LAST_METRIC      = "nh_ts_lst_metric";
        //! define the status of a node (loaded, initilized, deinitialized, started, stopped, unloaded)
        static const char * const NODE_HEALT_STATUS                     = "nh_status";
        //!last node error code
        static const char * const NODE_HEALT_LAST_ERROR_CODE            = "nh_lec";
        //!last node error message
        static const char * const NODE_HEALT_LAST_ERROR_MESSAGE         = "nh_lem";
        //!last node error domain
        static const char * const NODE_HEALT_LAST_ERROR_DOMAIN          = "nh_led";
    }
    /** @} */ // end of NodeHealtDefinitionKey
    
    /** @defgroup NodeHealtDefinitionValue !CHAOS node healt standard value
     *  @{
     */
    //! This namespace enclose all the value for the healt key
    namespace NodeHealtDefinitionValue {
        //! unloaded status
        static const char * const NODE_HEALT_STATUS_UNLOAD      = "Unload";
        //! unloaded status
        static const char * const NODE_HEALT_STATUS_UNLOADING   = "Unloading";
        //! load status
        static const char * const NODE_HEALT_STATUS_LOAD        = "Load";
        //! load status
        static const char * const NODE_HEALT_STATUS_LOADING     = "Loading";
        //! initilizeed status
        static const char * const NODE_HEALT_STATUS_INIT        = "Init";
        //! initilizeed status
        static const char * const NODE_HEALT_STATUS_INITING     = "Initializing";
        //! deinitialized status
        static const char * const NODE_HEALT_STATUS_DEINIT      = "Deinit";
        //! deinitialized status
        static const char * const NODE_HEALT_STATUS_DEINITING   = "Deinitializing";
        //! started status
        static const char * const NODE_HEALT_STATUS_START       = "Start";
        //! started status
        static const char * const NODE_HEALT_STATUS_STARTING    = "Starting";
        //! stopped status
        static const char * const NODE_HEALT_STATUS_STOP        = "Stop";
        //! stopped status
        static const char * const NODE_HEALT_STATUS_STOPING     = "Stoping";
        //! recoverable error status
        static const char * const NODE_HEALT_STATUS_RERROR      = "Recoverable Error";
        //! fatal error status
        static const char * const NODE_HEALT_STATUS_FERROR      = "Fatal Error";
    }
    /** @} */ // end of NodeHealtDefinitionValue
    
    /** @defgroup ControlUnitHealtDefinitionValue !CHAOS control unit specific key
     *  @{
     */
    //! This namespace enclose all the control unit specific healt key
    namespace ControlUnitHealtDefinitionValue {
        //!define the key that contains the rate of the output dataset pushes per second[double]
        static const char * const CU_HEALT_OUTPUT_DATASET_PUSH_RATE  = "cuh_dso_prate";

    }
    /** @} */ // end of NodeHealtDefinitionValue
    
    
    /** @defgroup NodeDomainAndActionRPC !CHAOS node rpc key description
     *  @{
     */
    //! Name space for grupping the key for action published by the node
    namespace NodeDomainAndActionRPC {
        //! The domain for the rpc action for every nodes (and sublcass)
        static const char * const RPC_DOMAIN                    = "system";
        //! Action that needs to answer with the status of the node(specialized for  every node)
        static const char * const ACTION_NODE_STATUS            = "ndk_rpc_request_status";
        //! Action that is called to inform the node of the registration status
        static const char * const ACTION_REGISTRATION_ACK        = "ndk_rpc_reg_ack";
        
        //! Start the control unit intialization, the action need the default value
        //! of the input attribute for a determinate device
        static const char * const ACTION_NODE_INIT                                  = "initNode";
        
        //! Deinitialization of a control unit, if it is in run, the stop phase
        //! is started befor deinitialization one
        static const char * const ACTION_NODE_DEINIT                                = "deinitNodeUnit";
        
        //! start the run method schedule for a determinated device
        static const char * const ACTION_NODE_START                                 = "startNodeUnit";
        
        //! pause the run method for a determinated device
        static const char * const ACTION_NODE_STOP                                  = "stopNodeUnit";
        
        //! recovery a recoverable state of the node
        static const char * const ACTION_NODE_RECOVERY                              = "recoveryNodeUnit";
        
        //! pause the run method for a determinated device
        static const char * const ACTION_NODE_RESTORE                               = "restoreNodeUnit";
        
        //! restore the control unit to a determinate temporal tag
        static const char * const ACTION_NODE_RESTORE_PARAM_TAG                     = "restoreNodeTag";
        
        //! is the name of the temporal tag to use as restore point
        static const char * const ACTION_NODE_GET_STATE                             = "getNodeState";
        
        //! return the control unit information
        static const char * const ACTION_CU_GET_INFO                                = "getNodeInfo";
        
        //! update the node property
        static const char * const ACTION_UPDATE_PROPERTY                            = "updateProperty";
        
    }
    /** @} */ // end of NodeDomainAndActionRPC
    
    /** @defgroup HealtProcessDefinitionKey !CHAOS healt process key description
     *  @{
     */
    namespace HealtProcessDefinitionKey {
    }
    /** @} */ // end of HealtProcessDefinitionKey
    
    /** @defgroup HealtProcessDomainAndActionRPC !CHAOS healt process key for rpc communication
     *  @{
     */
    namespace HealtProcessDomainAndActionRPC {
        //! The domain for unit server rpc action
        static const char * const RPC_DOMAIN                                        = "healt";
        //! This action is exposed by MDS and nned to be called to the remote node
        //! for publish itself
        static const char * const ACTION_PROCESS_WELCOME                            = "processHello";
        //! This action is exposed by MDS and nned to be called to the remote node
        //! for unpublish itself
        static const char * const ACTION_PROCESS_BYE                                = "processBye";
        
        
    }
    /** @} */ // end of HealtProcessDomainAndActionRPC
    
    /** @defgroup UnitServerNodeDefinitionKey !CHAOS unit server node key description
     *  @{
     */
    //! Name space for grupping key for the node type
    namespace UnitServerNodeDefinitionKey {
        
        //! key for the control unit aliases published by the unit server [array fo string]
        static const char * const UNIT_SERVER_HOSTED_CONTROL_UNIT_CLASS       = "usndk_hosted_cu_class";
        
        //! key the addresses the list of the states of the CU for a given Unit Serve   string]
        static const char * const UNIT_SERVER_HOSTED_CU_STATES                = "usndk_hosted_cu_states";
        
    }
    /** @} */ // end of UnitServerNodeDefinitionKey
    
    
    /** @defgroup UnitServerNodeDomainAndActionRPC !CHAOS unit server rpc key description
     *  This is the collection of all key used only by unit server
     *  @{
     */
    namespace UnitServerNodeDomainAndActionRPC {
        //! The domain for unit server rpc action
        static const char * const RPC_DOMAIN                                        = "unit_server";
        
        //! action called for load operation of the hosted control unit type
        static const char * const ACTION_UNIT_SERVER_LOAD_CONTROL_UNIT              = "unitServerLoadControlUnit";
        
        //! action called for load operation of the hosted control unit type
        static const char * const ACTION_UNIT_SERVER_UNLOAD_CONTROL_UNIT            = "unitServerUnloadControlUnit";
        
        //! Action called by mds for ack message in the unit server registration process
        static const char * const ACTION_UNIT_SERVER_REG_ACK                        = "unitServerRegistrationACK";
        
        //! Action that is called to inform the node of the registration status of the hosted control unit
        static const char * const ACTION_REGISTRATION_CONTROL_UNIT_ACK        = "ndk_rpc_reg_cu_ack";
        
        //! driver params passed during load operation for a specified control unit
        static const char * const PARAM_CU_LOAD_DRIVER_PARAMS                       = "controlUnitDriverParams";
        
        //! Alias to the intancer of the control unit to allocate [string]
        /*!
         Represent the control unit type to be load or unload
         */
        static const char * const PARAM_CONTROL_UNIT_TYPE                     = "usn_rpc_par_control_unit_type";
    }
    /** @} */ // end of UnitServerNodeDomainAndActionRPC
    
    
    /** @defgroup DataServiceNodeDefinitionKey !CHAOS data service node key description
     *  This is the collection of the key used to configure the DataProxy server
     *  address and port and how the client's(cu and ui) need to access to it (round robin or fail over)
     *  @{
     */
    //! This is the collection of the key to configura history and live channel
    namespace DataServiceNodeDefinitionKey {
        static const char * const DS_DIRECT_IO_FULL_ADDRESS_LIST                            = "dsndk_direct_io_full";
        //!lis the endpoitwhere is published the direct io[uint32_t]
        static const char * const DS_DIRECT_IO_ENDPOINT                        = "dsndk_direct_io_ep";
    }
    /** @} */ // end of DataServiceNodeDefinitionKey
    
    
    /** @defgroup DataServiceNodeDomainAndActionRPC !CHAOS data service rpc key description
     *  This is the collection of all key used only by unit server
     *  @{
     */
    namespace DataServiceNodeDomainAndActionRPC {
        //! The domain for unit server rpc action
        static const char * const RPC_DOMAIN                                    = "data_service";
    }
    /** @} */ // end of DataServiceNodeDomainAndActionRPC
    
    /** @defgroup ControlUnitNodeDefinitionKey List of control unit node type attribute key
     *  @{
     */
    //! Name space for grupping key for the control unit node type
    namespace ControlUnitNodeDefinitionKey {
        //! param to pass to the control unit during load operation[ string]
        static const char * const CONTROL_UNIT_LOAD_PARAM                           = "cudk_load_param";
        
        //! Description for the control unit dirvers [vector[string, string, string]*]
        static const char * const CONTROL_UNIT_DRIVER_DESCRIPTION                   = "cudk_driver_description";
        
        //! The name of the driver to use[strig]
        static const char * const CONTROL_UNIT_DRIVER_DESCRIPTION_NAME              = "cudk_driver_description_name";
        
        //! The version of the driver to use[strig]
        static const char * const CONTROL_UNIT_DRIVER_DESCRIPTION_VERSION           = "cudk_driver_description_version";
        
        //! The version of the driver to use[strig]
        static const char * const CONTROL_UNIT_DRIVER_DESCRIPTION_INIT_PARAMETER	= "cudk_driver_description_init_parameter";
        
        //!key for dataset description (array of per-attribute document)
        static const char * const CONTROL_UNIT_DATASET_DESCRIPTION                  = "cudk_ds_desc";
        
        //!key for dataset timestampt validity[uint64_t]
        static const char * const CONTROL_UNIT_DATASET_TIMESTAMP                    = "cudk_ds_timestamp";
        
        //!key for the name of dataset attribute
        static const char * const CONTROL_UNIT_DATASET_ATTRIBUTE_NAME               = "cudk_ds_attr_name";
        
        //!key representing the type of parameter
        static const char * const CONTROL_UNIT_DATASET_ATTRIBUTE_TYPE               = "cudk_ds_attr_type";
        
        //!key for the units ofr the attrbiute (ampere, volts)
        static const char * const CONTROL_UNIT_DATASET_ATTRIBUTE_UNIT               = "cudk_ds_attr_unit";
        
        //!key representing the name of the parameter
        static const char * const CONTROL_UNIT_DATASET_ATTRIBUTE_DESCRIPTION        = "cudk_ds_attr_desc";
        
        //!key representig the information for the parameter
        static const char * const CONTROL_UNIT_DATASET_ATTRIBUTE_DIRECTION          = "cudk_ds_attr_dir";
        
        //!key representing the value max size where need (type different from raw data type ex: int32)
        static const char * const CONTROL_UNIT_DATASET_VALUE_MAX_SIZE               = "cudk_ds_max_size";
        
        //!key representing the default value
        static const char * const CONTROL_UNIT_DATASET_DEFAULT_VALUE                = "cudk_default_value";
        
        //!key representing the default value
        static const char * const CONTROL_UNIT_DATASET_MAX_RANGE                    = "cudk_ds_max_range";
        
        //!key representing the default value
        static const char * const CONTROL_UNIT_DATASET_MIN_RANGE                    = "cudk_ds_min_range";
        
        //!key representing the bitmask flasgs of @DataType::DataTypeModfier that are applied on data type
        static const char * const CONTROL_UNIT_DATASET_MODIFIER                     = "cudk_ds_mod";
        
        //!key representing an array of type that describe the single or multiple subtype
        //that compese a single element whitin the binary data (int32 or array of int32)
        static const char * const CONTROL_UNIT_DATASET_BINARY_SUBTYPE               = "cudk_ds_bin_st";
        
        //!key representing how many times the binary subtype is repeated within the bunary data[int32]
        static const char * const CONTROL_UNIT_DATASET_BINARY_CARDINALITY           = "cudk_ds_bin_card";
        
        //!key representing the mime string associated to the MIME subtype
        static const char * const CONTROL_UNIT_DATASET_BINARY_MIME_DESCRIPTION      = "cudk_ds_bin_mime";
        
        //!key representing the standard mime type string, that describe the data
        //!within the binary field, associated to the MIME subtype
        static const char * const CONTROL_UNIT_DATASET_BINARY_MIME_ENCODING         = "cudk_ds_bin_encoding";
        
        //!The key represent an array with the object taht represent, each one, the command description array[object...]
        static const char * const CONTROL_UNIT_DATASET_COMMAND_DESCRIPTION         = "cudk_ds_command_description";
    }
    /** @} */ // end of ControlUnitNodeDefinitionKey
    
    namespace ControlUnitNodeDomainAndActionRPC {
        //!Alias associated to thefunction that apply the value changes set to the input dataset attribute
        static const char * const CONTROL_UNIT_APPLY_INPUT_DATASET_ATTRIBUTE_CHANGE_SET  = "cunrpc_ida_cs";
        
    }
    
    /** @defgroup Contorl unit system key
     *  This is the collection of the key representing the property that are exposed into system dataset
     *  @{
     */
    //! Name space for grupping control unit system property
    namespace ControlUnitDatapackSystemKey {
        //! represent the delay beetwen a subseguent cu start method call it is a property of a control unit
        static const char * const THREAD_SCHEDULE_DELAY                             = "cudk_thr_sch_delay";
    }
    /** @} */ // end of ControlUnitNodeDefinitionKey
    
    /** @defgroup CUType Control Unit Default Type
     *  This is the collection of the key for the classification of the control unit types
     *  @{
     */
    //! Name space for grupping control unit types
    namespace CUType {
        static const char * const RTCU	= "rtcu";
        static const char * const SCCU  = "sccu";
    }
    /** @} */ // end of CUType
    
    /** @defgroup ChaosDataType Chaos Basic datatype
     *  This is the collection of the definition of the chaos basic datatype
     *  @{
     */
    //! Name space for grupping the definition of the chaos basic datatype
    namespace DataType {
        //!typede for datatype
        typedef enum DataType {
            //!Integer 32 bit length
            TYPE_INT32 = 0,
            //!Integer 64 bit length
            TYPE_INT64,
            //!Double 64 bit length
            TYPE_DOUBLE,
            //!C string variable length
            TYPE_STRING,
            //!byte array variable length
            TYPE_BYTEARRAY,
            //!bool variable length
            TYPE_BOOLEAN,
            TYPE_CLUSTER,
            //!modifier to be ored to normal data types
            TYPE_ACCESS_ARRAY=0x100
            
        } DataType;
        
        typedef enum BinarySubtype {
            //!bool variable length
            SUB_TYPE_BOOLEAN = 0,
            //!Integer char bit length
            SUB_TYPE_CHAR,
            //!Integer 8 bit length
            SUB_TYPE_INT8,
            //!Integer 16 bit length
            SUB_TYPE_INT16,
            //!Integer 32 bit length
            SUB_TYPE_INT32,
            //!Integer 64 bit length
            SUB_TYPE_INT64,
            //!Double 64 bit length
            SUB_TYPE_DOUBLE,
            //!C string variable length
            SUB_TYPE_STRING,
            //! the subtype is represented by a specific mime type tagged in specific dataset constants
            SUB_TYPE_MIME,
            
            //! unsigned flag
            SUB_TYPE_UNSIGNED = 0x200,
        } BinarySubtype;
        
        typedef enum DataTypeModfier {
            //!bool variable length
            MODIFIER_UNSIGNED = 0
        } DataTypeModfier;
        
        //!define the direction of dataset element
        typedef enum DataSetAttributeIOAttribute{
            //!define an atribute in input
            Input = 0,
            //!define an atribute in output
            Output=1,
            //!define an atribute either two direction
            Bidirectional=2,
        } DataSetAttributeIOAttribute;
    }
    
    /** @defgroup CUStateKey Control Unit State
     *  This is the collection of the key for the control unit state definition
     *  @{
     */
    //! Name space for grupping option used for define the control unit state
    namespace CUStateKey {
        //! The state of the control unit
        static const char * const CONTROL_UNIT_STATE    = "cu_state";
        
        //!define states of the control unit
        /*!
         These enumeration represents the state of the control unit, the order match permit to map these to
         those of the stat machine internally defined into teh chaos::utility::StartableService.
         */
        typedef enum {
            //! define the node in uninitialized
            DEINIT  = 0,
            //! define the node is initialized and is ready to start
            INIT    = 1,
            //! define the node is running
            START   = 2,
            //! define the node has been stopped
            STOP    = 3,
            //!define an error state of the node, in error state the node wait until someone clear the error and put it again in START/STOP/DEINIT
            RECOVERABLE_ERROR = 4,
            //!define an error state of the node, in this case the error can't be recovered so it is equivalent to a deinit state
            FATAL_ERROR = 5,
            //!define the status of the node cannot be retrieved
            UNDEFINED
        } ControlUnitState;
    }
    /** @} */ // end of CUStateKey
    
    /** @defgroup MetadataServerNodeDefinitionKeyRPC List of mds node rpc action
     *  @{
     */
    //! Name space for grupping the key for action published by the mds node
    namespace MetadataServerNodeDefinitionKeyRPC {
        
        //! the key for the node registration[specific bson pack for every kind of node]
        static const char * const ACTION_REGISTER_NODE            = "mdsndk_rpc_a_reg_node";
        
        //! key that idetify the result of the node registration[int32]
        static const char * const PARAM_REGISTER_NODE_RESULT      = "mdsndk_rpc_p_reg_result";
    }
    /** @} */ // end of NodeDomainAndActionRPC
    
    
    /** @defgroup DataPackPrefixID Chaos Data Prefix
     This collection is a set for the prefix used for identify the domain
     for the unique id key in chaos data cloud
     @{
     */
    //! Namespace for the domain for the unique identification key
    namespace DataPackPrefixID {
        static const char * const OUTPUT_DATASE_PREFIX = "_o";
        static const char * const INPUT_DATASE_PREFIX = "_i";
        static const char * const CUSTOM_DATASE_PREFIX = "_c";
        static const char * const SYSTEM_DATASE_PREFIX = "_s";
        static const char * const HEALTH_DATASE_PREFIX = NodeHealtDefinitionKey::HEALT_KEY_POSTFIX;
    }
    /** @} */ // end of DataPackPrefixID
    
    /** @defgroup DataPackCommonKey Chaos Data Pack common key
     This is the collection of the common key that are contained into the
     all the dataset of a data producer
     @{
     */
    namespace DataPackCommonKey {
        //!define the device unique key, this represent the primary key of the producer[string]
        static const char * const DPCK_DEVICE_ID                       = chaos::NodeDefinitionKey::NODE_UNIQUE_ID;
        
        //!this define the acquisition timestamp of the data represented by the dataset[uint64_t]
        static const char * const DPCK_TIMESTAMP                       = "dpck_ats";//chaos::NodeDefinitionKey::NODE_TIMESTAMP;
        
        //!define the type of the dataset uint32_t [output(0) - input(1) - custom(2) - system(3) int32_t]
        static const char * const DPCK_DATASET_TYPE                    = "dpck_ds_type";
        //! the constant that represent the output dataset type
        static const unsigned int DPCK_DATASET_TYPE_OUTPUT             = 0;
        //! the constant that represent the input dataset type
        static const unsigned int DPCK_DATASET_TYPE_INPUT              = 1;
        //! the constant that represent the custom dataset type
        static const unsigned int DPCK_DATASET_TYPE_CUSTOM             = 2;
        //! the constant that represent the system dataset type
        static const unsigned int DPCK_DATASET_TYPE_SYSTEM             = 3;
    }
    /** @} */ // end of DataPackCommonKey
    
    /** @defgroup DataPackKey Chaos Data Pack output attirbute
     This is the collection of the standard key that are contained into the output
     attribute data pack that describe a producer state
     @{
     */
    //! Namespace for standard constant used for output attribute of a producer
    namespace DataPackOutputKey {
        //!this define key associated to the trigger
        static const char * const DPOK_TRIGGER_CODE                   = "dpok_trigger_key";
    }
    /** @} */ // end of DataPackKey
    
    /** @defgroup DataPackSystemKey Chaos Data Pack for System Attribute
     @{
     these are the stantdard key for chaos system attirbute
     */
    //! Namespace for standard constant used for system attribute
    namespace DataPackSystemKey{
        //!is the ehartbeat of a data producer
        //static const char * const DP_SYS_HEARTBEAT			= "dp_sys_hp";
        
        //!is the last error message occurred into data producer
        static const char * const DP_SYS_UNIT_TYPE			= "dp_sys_unit_type";
        
        //!is the last error occurred into the data producer
        static const char * const DP_SYS_LAST_ERROR			= "dp_sys_lerr";
        
        //!is the last error message occurred into data producer
        static const char * const DP_SYS_LAST_ERROR_MESSAGE	= "dp_sys_lerr_msg";
        
        //!is the domain where the last error has occurred into data producer
        static const char * const DP_SYS_LAST_ERROR_DOMAIN	= "dp_sys_lerr_domain";
        
        //!is the number of run unit
        static const char * const DP_SYS_RUN_UNIT_AVAILABLE	= "dp_sys_ru_available";
        
        //!is the run unit identifier
        static const char * const DP_SYS_RUN_UNIT_ID		= "dp_sys_ru_id";
        
    }
    /** @} */ // end of DataPackSystemKey
    
    
    
    /** @defgroup DatasetDefinitionkey Dataset definition key
     *  This is the collection of the key for the device dataset
     *  @{
     */
    //! Name space for grupping option used for define the dataset of the device abstraction
    namespace DatasetDefinitionkey {
        //!key for dataset descriptors array {[domain, name, paramteres....]}
        static const char * const DESCRIPTION                       = "ds_desc";
        //!key for dataset timestampt validity
        static const char * const TIMESTAMP                         = "ds_timestamp";
        //!key for the name of dataset attribute
        static const char * const ATTRIBUTE_NAME                    = "ds_attr_name";
        //!key for the tag of the dataset attrbiute
        static const char * const ATTRIBUTE_TAG                     = "ds_attr_tag";
        //!key representing the type of parameter
        static const char * const ATTRIBUTE_TYPE                    = "ds_attr_type";
        //!key for the units ofr the attrbiute(ampere, volts)
        static const char * const ATTRIBUTE_UNIT                    = "ds_attr_unit";
        //!key representing the name of the parameter
        static const char * const ATTRIBUTE_DESCRIPTION             = "ds_attr_desc";
        //!key representig the information for the parameter
        static const char * const ATTRIBUTE_DIRECTION               = "ds_attr_dir";
        //!key representing the value max size where need (type different from rawd data type ex: int32)
        static const char * const VALUE_MAX_SIZE                    = "ds_max_size";
        //!key representing the default value
        static const char * const DEFAULT_VALUE                     = "ds_default_value";
        //!key representing the default value
        static const char * const MAX_RANGE                         = "ds_max_range";
        //!key representing the default value
        static const char * const MIN_RANGE                         = "ds_min_range";
    }
    /** @} */ // end of DatasetDefinitionkey
    
    
    /** @defgroup ChaosErrorCode Chaos Error Code
     *  This is the collection of the definition of the chaos error code
     *  @{
     */
    //! Name space for grupping the definition of the chaos error code
    namespace ErrorCode {
        //!the list of principal chaos error code
        typedef enum {
            //!no error
            EC_NO_ERROR = 0,
            //! rpc timeout
            EC_TIMEOUT = 100,
            //! dataset attribute not found
            EC_ATTRIBUTE_NOT_FOUND, // 101 ...
            //! dataset attribute bad direction
            EC_ATTRIBUTE_BAD_DIR ,
            //!dataset attribute not supported
            EC_ATTRIBUTE_TYPE_NOT_SUPPORTED ,
            
            //!unit server registration is gone well
            EC_MDS_NODE_REGISTRATION_OK = 500,
            //!unit server registration has failed for invalid alias
            EC_MDS_NODE_REGISTRATION_FAILURE_INVALID_ALIAS,
            //!unit server registration for duplicated alias
            EC_MDS_NODE_REGISTRATION_FAILURE_DUPLICATE_ALIAS,
            //! node bad state machine state in response to mds ack event
            EC_MDS_NODE_BAD_SM_STATE,
            //!work unit is not self manageable and need to be loaded within an unit server
            EC_MDS_NODE_ID_NOT_SELF_MANAGEABLE
        } ErrorCode;
    }
    /** @} */ // end of ChaosDataType
    
    /** @} */ // end of ChaosDataType
    
    /** @defgroup RpcActionDefinitionKey Action Rpc Protocol key
     *  This is the collection of the key used for the intere rpc protocol
     *  @{
     */
    //! Name space for grupping option used for define the custom action to share via RPC !CHAOS system
    namespace RpcActionDefinitionKey {
        //!command manager rpc tag, this is the tag that rpc subsystem must to use to transfer BSON package between chaos node rpc endpoint
        static const char * const CS_CMDM_RPC_TAG                             = "chaos_rpc";
        //!key for action domain descriptors array {[domain, name, paramteres....]}
        static const char * const CS_CMDM_ACTION_DESC                         = "act_desc";
        
        //!key for the array of object that represent the action paramteres
        //!description
        static const char * const CS_CMDM_ACTION_DESC_PARAM                   = "act_desc_param";
        
        //!key representing the name of the parameter
        static const char * const CS_CMDM_ACTION_DESC_PAR_NAME                = "act_desc_par_name";
        
        //!key representig the information for the parameter
        static const char * const CS_CMDM_ACTION_DESC_PAR_INFO                = "act_desc_par_info";
        
        //!key representing the type of parameter
        static const char * const CS_CMDM_ACTION_DESC_PAR_TYPE                = "act_desc_par_type";
        
        //!comamnd description constant
        //!key for action domain definition
        static const char * const CS_CMDM_ACTION_DOMAIN                       = "act_domain";
        
        //!key for action name definition
        static const char * const CS_CMDM_ACTION_NAME                         = "act_name";
        
        //!key for action message
        static const char * const CS_CMDM_ACTION_MESSAGE                      = "act_msg";
        
        //!key for the specify the id of the request(the current message is an asnwer)
        static const char * const CS_CMDM_MESSAGE_ID                          = "act_msg_id";
        
        //!key for action name definition
        static const char * const CS_CMDM_ACTION_DESCRIPTION                  = "act_desc";
        
        
        //!key action submission result
        //static const char * const CS_CMDM_ACTION_SUBMISSION_RESULT            = "act_sub_result";
        
        //!key action submission error message
        static const char * const CS_CMDM_ACTION_SUBMISSION_ERROR_MESSAGE     = "act_sub_emsg";
        
        //!key action submission error domain
        static const char * const CS_CMDM_ACTION_SUBMISSION_ERROR_DOMAIN      = "act_sub_edom";
        
        //!key action submission error code
        static const char * const CS_CMDM_ACTION_SUBMISSION_ERROR_CODE        = "act_sub_ecode";
        
        //!key action submission error server address
        /*!
         Identify the address of the server that can be reached or that don't have sent the ack
         message
         */
        static const char * const CS_CMDM_ACTION_SUBMISSION_ERROR_SERVER_ADDR = "act_sub_e_srv_addr";
        
        //!key for action sub command description
        static const char * const CS_CMDM_SUB_CMD                             = "sub_cmd";
        
        //!key for the ip where to send the answer
        static const char * const CS_CMDM_ANSWER_HOST_IP                      = "rh_ans_ip";
        //!key for the ip where to send the answer
        static const char * const CS_CMDM_ANSWER_DOMAIN                      = "rh_ans_domain";
        //!key for the ip where to send the answer
        static const char * const CS_CMDM_ANSWER_ACTION                      = "rh_ans_action";
        //!key for the answer, it is needed byt the requester to recognize the answer
        static const char * const CS_CMDM_ANSWER_ID                          = "rh_ans_msg_id";
        //!ker ofr the ip where to send the rpc pack
        static const char * const CS_CMDM_REMOTE_HOST_IP                     = "rh_ip";
    }
    /** @} */ // end of RpcActionDefinitionKey
    
    
    /** @defgroup ChaosSystemDomainAndActionLabel Chaos System Action Label
     *  This is the collection of the label that identify the name of the action defined at system level(doman "system")
     *  These are the action that are  managed by meta data service to permit the base interaction with other node.
     *  Every node need to register itself through mds for interact with other !CHAOS node
     *  @{
     */
    namespace ChaosSystemDomainAndActionLabel {
        //! The chaos action domain for system message
        static const char * const SYSTEM_DOMAIN									= "system";
        
        //! Action to retrive all device id
        static const char * const MDS_GET_ALL_DEVICE							= "getAllActiveDevice";
        
        //! Perform the heart beat of the cu
        static const char * const MDS_CU_HEARTBEAT								= "heartbeatControlUnit";
        
        //! Perform request of the network address for a node identified by a device id
        static const char * const MDS_GET_NODE_ADDRESS							= "getNodeNetworkAddress";
        
        //! This action provide to the shutdown porcess of the enteir daemon
        //! that runt the active contorl units. All it will be gracefull shutten down
        //! before daemon exit
        static const char * const ACTION_SYSTEM_SHUTDOWN                        = "shutdownUnitServer";
        
        
        //! key for the server alias used by the instance [string]
        static const char * const UNIT_SERVER_STATES_ANSWER                     = "unit_server_states";
        
        //! Action called by mds for ack message in the unit server registration process
        static const char * const ACTION_NODE_REG_ACK                           = "nodeRegistrationACK";
    }
    /** @} */ // end of ChaosSystemDomainAndActionLabel
    
    /** @defgroup PerformanceSystemRpcKey Chaos performance system
     * this is the collection of the rpc key for interacting with
     * internal performance system
     *  @{
     */
    namespace PerformanceSystemRpcKey {
        //-------------------------performance-----------------------
        //! The chaos action domain for system message
        static const char * const SYSTEM_PERFORMANCE_DOMAIN = "system:perf";
        
        static const char * const ACTION_PERFORMANCE_INIT_SESSION= "sp:init_session";
        
        static const char * const ACTION_PERFORMANCE_CLOSE_SESSION= "sp:close_session";
        
        static const char * const KEY_REQUEST_SERVER_DESCRITPION = "sp::req_serv_desc";
    }
    /** @} */ // end of PerformanceSystemRpcKey
    
    
    namespace event {
        /** @defgroup EventConfiguration Chaos event constant for server
         and cleint configuration
         @{
         */
        //! Name space for grupping option used for commandline or in config file
        namespace EventConfiguration {
            //!  for choice the implementation
            static const char * const   OPTION_KEY_EVENT_ADAPTER_IMPLEMENTATION     = "evt_adpt_impl";
            //! @Configuraiton for alert event multicast ip
            static const char * const   CONF_EVENT_ALERT_MADDRESS                   = "239.255.0.1";
            //! @Configuraiton for instruments event multicast ip
            static const char * const   CONF_EVENT_INSTRUMENT_MADDRESS              = "239.255.0.2";
            //! @Configuraiton for command event multicast ip
            static const char * const   CONF_EVENT_COMMAND_MADDRESS                 = "239.255.0.3";
            //! @Configuraiton for custom event multicast ip
            static const char * const   CONF_EVENT_CUSTOM_MADDRESS                  = "239.255.0.4";
            //! @Configuraiton for event multicast ip port
            static const unsigned short CONF_EVENT_PORT                             = 5000;
        }
        /** @} */ // end of EventConfiguration
    }
    
    /*
     * separator to be used in node naming 
     */
    static const char PATH_SEPARATOR                                                ='/';
}
#endif
