/*
 * Copyright 2022 INFN
 *
 * Andrea Michelotti
 * Licensed under the EUPL, Version 1.2 or – as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */
#ifndef __ReadWriteInterface_H__
#define __ReadWriteInterface_H__
#include "AbstractDriver.h"
namespace chaos {
    namespace cu  {
        namespace driver_manager {
            namespace driver {


                class ReadWriteInterface: public chaos::cu::driver_manager::driver::AbstractDriver  {
                
                public:
                    virtual void driverInit(const char *initParameter){}
                    virtual void driverDeinit(){}
                    /**
                     \brief Read  from the physical device
                     \param buffer[out] destination buffer
                     \param addr[in]  address or identification
                     \param bcout[in] buffer count
                     \return the number of succesful read items, negative error

                     */
                    virtual int read(void *buffer, int addr, int bcount) = 0;
                    /**
                     \brief Write a channel of the device
                     \param buffer[out] destination buffer
                     \param addr[in] channel address or identification
                     \param bcout[in] buffer count
                     \return the number of succesful written items, negative error
                     */
                    virtual int write(void *buffer, int addr, int bcount) = 0;

                    /**
                     \brief Initialize the specific driver
                     \param buffer[in] initialisation opaque parameter
                     \return 0 if success, error otherwise
     
                     */
                    virtual int initIO(void *buffer, int sizeb) = 0;

                    /**
                     \brief deinit the driver
                     \param buffer[in] initialisation opaque parameter
                     \return 0 if success, error otherwise
                     */
                    virtual int deinitIO() = 0;
                    
                    
                    /**
                     \brief perform a specific I/O operation  the device
                     \param data[inout] operation data opaque parameter
                     \param sizeb[in] operation data size/maxsize
                     \return 0 if success, error otherwise
                     */
                    virtual int iop(int operation,void*data,int sizeb)=0;
                   
                };
            }

        }
    }
}
#endif 
