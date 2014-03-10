//
//  DirectIODeviceChannelGlobal.h
//  CHAOSFramework
//
//  Created by Claudio Bisegni on 18/02/14.
//  Copyright (c) 2014 INFN. All rights reserved.
//

#ifndef CHAOSFramework_DirectIODeviceChannelGlobal_h
#define CHAOSFramework_DirectIODeviceChannelGlobal_h

#include <string>
#include <stdint.h>
#include <arpa/inet.h>

namespace chaos {
	namespace common {
		namespace direct_io {
			namespace channel {
				
				namespace opcode {
                    
					/*!
                     \enum DeviceChannelOpcode
                     Opcode used by the DirectIO device channel
					 */
					typedef enum DeviceChannelOpcode {
						DeviceChannelOpcodePutOutput				= 1,	/**< send the output dataset */
						DeviceChannelOpcodeGetLastOutput            = 2,	/**< request the output dataset*/
						DeviceChannelOpcodePutInput					= 4,	/**< send the input dataset */
						DeviceChannelOpcodePutNewReceivedCommand	= 8	/**< send over the channel the received command */
					} DeviceChannelOpcode;
				}
				
                
                //! Name space for grupping the varius headers for every DeviceChannelOpcode
                namespace opcode_headers {
                    
                    //! Heder for the DeviceChannelOpcodePutOutput[WithCache] opcodes
                    typedef struct DirectIODeviceChannelHeaderPutOpcode {
							//! The 32bit hash value for the device that we need to insert
                        uint32_t device_hash;
                            //! The 32bit cache_tag
                        uint32_t cache_tag;
                    } DirectIODeviceChannelHeaderData, *DirectIODeviceChannelHeaderDataPtr;
                    
                    //! Heder for the DeviceChannelOpcodeGetOutputFromCache opcode
                    typedef	union DirectIODeviceChannelHeaderGetOpcode {
                        //raw data representation of the header
                        char raw_data[18];
                        struct header {
							//! The endpoint where the channel is published
                            uint16_t	endpoint;
							//! The priority port value for the device that we need to get
                            uint16_t	p_port;
							//! The priority port value for the device that we need to get
                            uint16_t	s_port;
                                //! The 32bit hash value for the device that we need to get
                            uint32_t	device_hash;
                                //! The 32bit representation for the ip where send the answer
                            uint64_t	address;
                        } field;
                    } DirectIODeviceChannelHeaderGetOpcode, *DirectIODeviceChannelHeaderGetOpcodePtr;
                }
			}
		}
	}
}

#endif
