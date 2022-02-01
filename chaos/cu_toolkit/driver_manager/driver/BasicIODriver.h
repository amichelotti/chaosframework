/*
 * Copyright 2012, 2017 INFN
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
#ifndef __BASICIODRIVER_H__
#define __BASICIODRIVER_H__
#include "AbstractDriverPlugin.h"
#include "ReadWriteInterface.h"
namespace chaos {
    namespace cu  {
        namespace driver_manager {
            namespace driver {

/*
                 driver definition
                 */

                class BasicIODriver : public chaos::cu::driver_manager::driver::AbstractDriverPlugin,public ReadWriteInterface {
                    void driverInit(const char *initParameter);
                    void driverDeinit();
                protected:
                    /*
                    ddDataSet_t *dataset;
                    int datasetSize;
                    */
                public:
                    BasicIODriver();

                    ~BasicIODriver();
                    //! Execute a command
                    chaos::cu::driver_manager::driver::MsgManagmentResultType::MsgManagmentResult execOpcode(chaos::cu::driver_manager::driver::DrvMsgPtr cmd);
                   
                    virtual int iop(int operation,void*data,int sizeb);
                    /**
                       \brief return the size in byte of the dataset
                       \return the size of the dataset if success, zero otherwise
                     */
                 //   int getDatasetSize();

                    /**
                       \brief return the dataset copying max size bytes
                       \param data[out] array of data
                       \param sizeb[in] max byte to copy
                       \return the size of the dataset if success, zero otherwise
                     */

                  //  int getDataset(ddDataSet_t*data, int sizeb);

                  //  void setDataSet(ddDataSet_t*data, int sizeb);
                };
            }

        }
    }
}
#endif 
