#include "AbstractDriverInterface.h"

namespace chaos {

namespace cu {
namespace driver_manager{
namespace driver{

 chaos::common::data::CDWUniquePtr AbstractDriverInterface::getDrvProperties(){return accessor->getDrvProperties();}


                    /**
                     * @brief set a CDataWrapper (JSON) with the optional properties 
                     * 
                     * @return properties
                     */

    chaos::common::data::CDWUniquePtr AbstractDriverInterface::setDrvProperties(chaos::common::data::CDWUniquePtr &prop){
        return accessor->setDrvProperties(prop);
        }

    std::string AbstractDriverInterface::getLastError(){return accessor->getLastError();}
    AbstractDriverInterface::~AbstractDriverInterface(){
    }


}}}}