/*
 * Copyright 2012, 2017 INFN
 *
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

#include <chaos/common/utility/TimingUtil.h>
#include <chaos/common/configuration/GlobalConfiguration.h>

#include <chaos/common/event/channel/InstrumentEventChannel.h>
#include <chaos/cu_toolkit/control_manager/RTProcessAbstractControlUnit.h>

using namespace chaos;
using namespace chaos::common::data;
using namespace chaos::common::utility;
using namespace chaos::common::exception;
using namespace chaos::common::data::cache;

using namespace chaos::cu;
using namespace chaos::cu::control_manager;

#define RTCUL_HEAD "[RT Process Control Unit:"<<getCUInstance()<<"] - "
#define RTCULAPP_ LAPP_ << RTCUL_HEAD
#define RTCULDBG_ LDBG_ << RTCUL_HEAD
#define RTCULERR_ LERR_ << RTCUL_HEAD << __PRETTY_FUNCTION__ <<"(" << __LINE__ << ") - "


 void RTProcessAbstractControlUnit::parseSubscriptions(const std::string& load_params){
    CDataWrapper p;
    subscribe_nodes.clear();
    if(load_params.size()==0){
        return;
    }
    try {
        p.setSerializedJsonData(load_params.c_str());
        if(p.hasKey(CONTROL_UNIT_CONSUMER_GROUP_KEY)){
            gid=p.getStringValue(CONTROL_UNIT_CONSUMER_GROUP_KEY);
        }
        if(p.hasKey(CONTROL_UNIT_SUBSCRIPTIONS_KEY)){
            if(p.isStringValue(CONTROL_UNIT_SUBSCRIPTIONS_KEY)){
                subscribe_nodes.insert(p.getStringValue(CONTROL_UNIT_SUBSCRIPTIONS_KEY));
            } else if(p.isVectorValue(CONTROL_UNIT_SUBSCRIPTIONS_KEY)){
                CMultiTypeDataArrayWrapperSPtr v=p.getVectorValue(CONTROL_UNIT_SUBSCRIPTIONS_KEY);
                for(int cnt=0;cnt<v->size();cnt++){
                        subscribe_nodes.insert(v->getStringElementAtIndex(cnt));
                            
                    }
            }
        } 
        
        if(p.hasKey(CONTROL_UNIT_DISCARD_TOO_OLD_KEY)){
            discard_too_old=p.getInt32Value(CONTROL_UNIT_DISCARD_TOO_OLD_KEY);
        }
    } catch(...){

    }
    if(subscribe_nodes.size()==0){
        throw CException(-1, "subscribed node list empty", __PRETTY_FUNCTION__);

    }
    
 }
RTProcessAbstractControlUnit::RTProcessAbstractControlUnit(const std::string& _control_unit_id,
                                             const std::string& _control_unit_param):
AbstractControlUnit(CUType::RTCU,
                    _control_unit_id,
                    _control_unit_param),
schedule_delay(1),consumer(NULL),discard_too_old(0),
scheduler_run(false){
    //allocate the handler engine
    // attributeHandlerEngine = new DSAttributeHandlerExecutionEngine(this);
    gid=_control_unit_id;
    //associate the shared cache of the executor to the asbtract control unit one
    attribute_value_shared_cache = new AttributeValueSharedCache();
    parseSubscriptions(_control_unit_param);
}

/*!
 Parametrized constructor
 \param _control_unit_id unique id for the control unit
 \param _control_unit_drivers driver information
 */
RTProcessAbstractControlUnit::RTProcessAbstractControlUnit(const std::string& _control_unit_id,
                                             const std::string& _control_unit_param,
                                             const ControlUnitDriverList& _control_unit_drivers):
AbstractControlUnit(CUType::RTCU,
                    _control_unit_id,
                    _control_unit_param,
                    _control_unit_drivers),
schedule_delay(1),consumer(NULL),discard_too_old(0),
scheduler_run(false) {
    //allocate the handler engine
    //attributeHandlerEngine = new DSAttributeHandlerExecutionEngine(this);
    gid=_control_unit_id;

    //associate the shared cache of the executor to the asbtract control unit one
    attribute_value_shared_cache = new AttributeValueSharedCache();
    parseSubscriptions(_control_unit_param);

}


RTProcessAbstractControlUnit::RTProcessAbstractControlUnit(const std::string& _alternate_type,
                                             const std::string& _control_unit_id,
                                             const std::string& _control_unit_param):
AbstractControlUnit(_alternate_type,
                    _control_unit_id,
                    _control_unit_param),
schedule_delay(1),consumer(NULL),discard_too_old(0),
scheduler_run(false) {
    gid=_control_unit_id;

    //associate the shared cache of the executor to the asbtract control unit one
    attribute_value_shared_cache = new AttributeValueSharedCache();
    parseSubscriptions(_control_unit_param);

}

RTProcessAbstractControlUnit::RTProcessAbstractControlUnit(const std::string& _alternate_type,
                                             const std::string& _control_unit_id,
                                             const std::string& _control_unit_param,
                                             const ControlUnitDriverList& _control_unit_drivers):
AbstractControlUnit(_alternate_type,
                    _control_unit_id,
                    _control_unit_param,
                    _control_unit_drivers),
schedule_delay(1),consumer(NULL),discard_too_old(0),
scheduler_run(false) {
    gid=_control_unit_id;

    //associate the shared cache of the executor to the asbtract control unit one
    attribute_value_shared_cache = new AttributeValueSharedCache();
    parseSubscriptions(_control_unit_param);

}

RTProcessAbstractControlUnit::~RTProcessAbstractControlUnit() {
    //release attribute shared cache
    if(attribute_value_shared_cache) {
        delete(attribute_value_shared_cache);
    }
}

void RTProcessAbstractControlUnit::setDefaultScheduleDelay(uint64_t _schedule_delay) {
    
    RTCULDBG_<<"setting default schedule to:"<<_schedule_delay<<" us";
    schedule_delay = _schedule_delay;
}

/*
 fill the CDataWrapper with AbstractCU system configuration, this method
 is called after getStartConfiguration directly by sandbox. in this method
 are defined the action for the input element of the dataset
 */
void RTProcessAbstractControlUnit::_defineActionAndDataset(CDataWrapper& setup_configuration)  {
    AbstractControlUnit::_defineActionAndDataset(setup_configuration);
}

AbstractSharedDomainCache *RTProcessAbstractControlUnit::_getAttributeCache() {
    return AbstractControlUnit::_getAttributeCache();
}

void RTProcessAbstractControlUnit::consumer_handler(chaos::common::message::ele_t&data){
    if(data.cd.get()==NULL){
        RTCULERR_<<"Empty packet from key:"<<data.key<<" off:"<<data.off<<" partition:"<<data.par;
        return;
    }
    if((discard_too_old>0) && data.cd->hasKey(DataPackCommonKey::DPCK_TIMESTAMP)){
        int64_t now=chaos::common::utility::TimingUtil::getTimeStamp();
        int64_t devt=data.cd->getInt64Value(DataPackCommonKey::DPCK_TIMESTAMP) ;
        int64_t diff=now - devt;
        if(diff>discard_too_old){
            // skip packet
            RTCULDBG_<<"skip packet of "<<data.key<<" of "<<diff<<" ms ago ("<<chaos::common::utility::TimingUtil::toString(devt)<< "), older than "<<discard_too_old<< " ms";
            return;
        }
    }
    try{
            //! exec the control unit step
            unitProcessData(data.key,data.cd);
        } catch(CException& ex) {
            //go in recoverable error
            boost::thread(boost::bind(&AbstractControlUnit::_goInRecoverableError, this, ex)).detach();
            boost::this_thread::sleep_for(boost::chrono::seconds(2));
        }  catch(...) {
            CException ex(-1, "undefined error", __PRETTY_FUNCTION__);
            //go in recoverable error
            boost::thread(boost::bind(&AbstractControlUnit::_goInRecoverableError, this, ex)).detach();
            boost::this_thread::sleep_for(boost::chrono::seconds(2));
        }

        //udpate output dataset timestamp tag
    _updateAcquistionTimestamp((uint64_t)TimingUtil::getTimeStampInMicroseconds());
    pushOutputDataset();
}
void RTProcessAbstractControlUnit::unitProcessData(std::string& key,chaos::common::data::CDWUniquePtr& cd){
    RTCULDBG_ <<" Must be overloaded, data from "<<key<<" ds size:"<<cd->getBSONRawSize();
}
/*!
 Init the  RT Control Unit scheduling for device
 */
void RTProcessAbstractControlUnit::init(void *initData) {
    //call parent impl
    AbstractControlUnit::init(initData);
    
    scheduler_run = false;
    
    RTCULDBG_ << "Initializing shared attribute cache " << DatasetDB::getDeviceID();
    InizializableService::initImplementation((AttributeValueSharedCache*)attribute_value_shared_cache, (void*)NULL, "attribute_value_shared_cache", __PRETTY_FUNCTION__);
    std::string msgbrokerdrv = GlobalConfiguration::getInstance()->getOption<std::string>(InitOption::OPT_MSG_BROKER_DRIVER);

    std::string msgbroker =GlobalConfiguration::getInstance()->getOption<std::string>(InitOption::OPT_MSG_BROKER_SERVER);


    consumer = chaos::common::message::MessagePSDriver::getNewConsumerDriver(msgbrokerdrv, gid);
    if(consumer.get()){
        consumer->addServer(msgbroker);
        RTCULDBG_ << "New Consumer Broker:" << msgbroker<<" gid:"<<gid;

        consumer->addHandler(chaos::common::message::MessagePublishSubscribeBase::ONARRIVE, boost::bind(&RTProcessAbstractControlUnit::consumer_handler, this, _1));
         if (consumer->applyConfiguration() != 0) {
            throw chaos::CException(-1, "cannot initialize Publish Subscribe:" + consumer->getLastError(), __PRETTY_FUNCTION__);
        }
    }
    
}

/*!
 Starto the  Control Unit scheduling for device
 */
void RTProcessAbstractControlUnit::start() {

    //call parent impl
    AbstractControlUnit::start();
    for(std::set<std::string>::iterator i =subscribe_nodes.begin();i!=subscribe_nodes.end();i++){
        RTCULDBG_ << "Subscribing to:" << *i;

        consumer->subscribe(*i,true);

    }
   
    consumer->start();
}

/*!
 Stop the Custom Control Unit scheduling for device
 */
void RTProcessAbstractControlUnit::stop() {
    //manage the thread
    consumer->stop();

}

/*!
 Init the  RT Control Unit scheduling for device
 */
void RTProcessAbstractControlUnit::deinit() {
    //call parent impl
    AbstractControlUnit::deinit();
    for(std::set<std::string>::iterator i =subscribe_nodes.begin();i!=subscribe_nodes.end();i++){
        RTCULAPP_ << "Un Subscribing from:" << *i;

        consumer->subscribe(*i,false);

    }
    consumer->stop();

    InizializableService::deinitImplementation((AttributeValueSharedCache*)attribute_value_shared_cache, "attribute_value_shared_cache", __PRETTY_FUNCTION__);
}

void RTProcessAbstractControlUnit::unitInit(){
    RTCULDBG_ << "Init";


}
void RTProcessAbstractControlUnit::unitStart(){
        RTCULDBG_ << "Start";


}
void RTProcessAbstractControlUnit::unitStop(){
            RTCULDBG_ << "Stop";


}
void RTProcessAbstractControlUnit::unitDeinit(){
            RTCULDBG_ << "unitDeinit";


}

