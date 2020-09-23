#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/global.h>
#ifndef __DATAPROPERTY__
#define __DATAPROPERTY__
namespace chaos {
namespace common {
namespace data {

#define CREATE_DRV_INT_PROP(n,pub,var,min,max,inc,typ) \
createProperty(n,var,(int32_t)min,(int32_t)max,(int32_t)inc,pub,[](AbstractDriver*thi,const std::string&name,\
      const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addInt32Value("value",((typ*)thi)->var);\
        return ret;\
      },[](AbstractDriver*thi,const std::string&name,\
       const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getInt32Value("value");\
          return p.clone();});

#define CREATE_DRV_DOUBLE_PROP(n,pub,var,min,max,inc,typ) \
createProperty(n,var,min,max,inc,pub,[](AbstractDriver*thi,const std::string&name,\
      const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addDoubleValue("value",((typ*)thi)->var);\
        return ret;\
      },[](AbstractDriver*thi,const std::string&name,\
       const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getDoubleValue("value");\
          return p.clone();});

#define CREATE_DRV_BOOL_PROP(n,pub,var,typ) \
createProperty(n,var,pub,[](AbstractDriver*thi,const std::string&name,\
      const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addBoolValue("value",((typ*)thi)->var);\
        return ret;\
      },[](AbstractDriver*thi,const std::string&name,\
       const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getBoolValue("value");\
          return p.clone();});

#define CREATE_DRV_STRING_PROP(n,pub,var,typ) \
createProperty(n,var,pub,[](AbstractDriver*thi,const std::string&name,\
      const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addStringValue("value",((typ*)thi)->var);\
        return ret;\
      },[](AbstractDriver*thi,const std::string&name,\
       const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getStringValue("value");\
          return p.clone();});

#define CREATE_CU_INT_PROP(n,pub,var,min,max,inc,typ) \
createProperty(n,var,(int32_t)min,(int32_t)max,(int32_t)inc,pub,[](AbstractControlUnit*thi,const std::string&name,\
      const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addInt32Value("value",((typ*)thi)->var);\
        return ret;\
      },[](AbstractControlUnit*thi,const std::string&name,\
       const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getInt32Value("value");\
          return p.clone();});

#define CREATE_CU_DOUBLE_PROP(n,pub,var,min,max,inc,typ) \
createProperty(n,var,min,max,inc,pub,[](AbstractControlUnit*thi,const std::string&name,\
      const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addDoubleValue("value",((typ*)thi)->var);\
        return ret;\
      },[](AbstractControlUnit*thi,const std::string&name,\
       const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getDoubleValue("value");\
          return p.clone();});

#define CREATE_CU_BOOL_PROP(n,pub,var,typ) \
createProperty(n,var,pub,[](AbstractControlUnit*thi,const std::string&name,\
      const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addBoolValue("value",((typ*)thi)->var);\
        return ret;\
      },[](AbstractControlUnit*thi,const std::string&name,\
       const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getBoolValue("value");\
          return p.clone();});


#define CREATE_CU_STRING_PROP(n,pub,var,typ) \
createProperty(n,var,pub,[](AbstractControlUnit*thi,const std::string&name,\
      const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addStringValue("value",((typ*)thi)->var);\
        return ret;\
      },[](AbstractControlUnit*thi,const std::string&name,\
       const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getStringValue("value");\
          return p.clone();});

#define FILLPROPERTYCD(cw, value, min, max, incr)                              \
  cw->append("value", value);                                                  \
  cw->append("min", min);                                                      \
  cw->append("max", max);                                                      \
  cw->append("incr", incr);
/**
 * Class to store properties/variables that can be abstracted and private
 * typic example is in the use of camera properties that has to be normalized
 * for an abstracted use
 */
template <typename BC> class Property {
  chaos::common::data::CDataWrapper props;
  typedef  chaos::common::data::CDWUniquePtr (*conversion_func_t)(
      BC *, const std::string &name, const chaos::common::data::CDataWrapper &);
  std::map<std::string, std::string> abstract2props;
  std::map<std::string, conversion_func_t> prop2setHandler;
  std::map<std::string, conversion_func_t> prop2getHandler;
  boost::mutex lock;
public:
  void reset() { props.reset(); }
  chaos::common::data::CDWUniquePtr getProperties(bool sync = false) {
    if (sync) {
      syncRead();
    }
    return props.clone();
  }
  void importKeysAsProperties(chaos::common::data::CDataWrapper &p,bool sync=false) {
    ChaosStringVector sv;
    p.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
        chaos::common::data::CDataWrapper cd;
        p.copyKeyToNewKey(*i,"value",cd);
        setProperty(*i,cd,sync);
    }
  }

  void appendPropertiesTo(chaos::common::data::CDataWrapper &p) {
    ChaosStringVector sv;
    props.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
      chaos::common::data::CDataWrapper cd;
      if (props.isCDataWrapperValue(*i)) {
        props.getCSDataValue(*i, cd);
        if(cd.hasKey("value")){
          p.copyKeyToNewKey("value",*i,p);
      }
    }
  }
  }
  void appendPubPropertiesTo(chaos::common::data::CDataWrapper &p) {
    ChaosStringVector sv;
    props.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
      chaos::common::data::CDataWrapper cd;
      if (props.isCDataWrapperValue(*i)) {
        props.getCSDataValue(*i, cd);
        if(cd.hasKey("pubname")&&cd.isStringValue("pubname")){
          std::string pub=cd.getStringValue("pubname");
          if((pub.size()>0)&&cd.hasKey("value")){
            cd.copyKeyToNewKey("value",pub,p);
          }

        }
      }
    }
  }
  bool hasKey(const std::string &key) { return props.hasKey(key); }

  // between public and private properties
/**
 * @brief Create a Property by using the getHandler
 * 
 * @param pubname 
 * @param getHandler 
 * @param setHandler 
 * @return chaos::common::data::CDWUniquePtr 
 */
chaos::common::data::CDWUniquePtr createProperty(
    const std::string propname, 
    conversion_func_t getHandler,
    conversion_func_t setHandler,
    const std::string &pubname = "") {
      
    if (setHandler) {
        prop2setHandler[propname] = setHandler;
      }
      if (getHandler) {
        prop2getHandler[propname] = getHandler;
      } else {
        throw chaos::CException(-10,"Get handler of "+propname+" must be set",__FUNCTION__);

      }
    chaos::common::data::CDWUniquePtr ret=getHandler((BC *)this,propname, chaos::common::data::CDataWrapper());
    if (ret.get()==NULL) {
      LERR_<<"Cannot initialize property "+propname+ " from get callback";
        return chaos::common::data::CDWUniquePtr();
    }
    if (!props.hasKey(propname)) {
      if (pubname.size() > 0) {
        ret->append("pubname", pubname);
        abstract2props[pubname] = propname;
      }
      if(ret.get()){
        props.append(propname,*ret.get());

      }

    } else {

      setProperty(propname,*ret.get());
    }
    return props.clone();
  }

  chaos::common::data::CDWUniquePtr createProperty(
      const std::string &propname, chaos::common::data::CDataWrapper &value,
      const std::string &pubname = "", conversion_func_t getHandler = NULL,
      conversion_func_t setHandler = NULL) {
    if (setHandler) {
        prop2setHandler[propname] = setHandler;
      }
      if (getHandler) {
        prop2getHandler[propname] = getHandler;
      }
    if (!props.hasKey(propname)) {
      if (pubname.size() > 0) {
        value.append("pubname", pubname);
        abstract2props[pubname] = propname;
      }
     if(!value.hasKey("value")){
       throw chaos::CException(-2,propname+" missing required key 'value",__FUNCTION__);
     }
      props.append(propname,value);

      syncWrite(propname);

    } else {
      setProperty(propname,value);
     // props.setValue(propname,
       //              (const chaos::common::data::CDataWrapper *)&value);
       
    }
    return props.clone();
  }

  chaos::common::data::CDWUniquePtr retriveProp(std::string &propname) {
    if (props.hasKey(propname) && props.isCDataWrapperValue(propname)) {
   //   LDBG_<<__FUNCTION__<<" -0 retrive prop full before:"<<props.getJSONString();

      chaos::common::data::CDWUniquePtr p=props.getCSDataValue(propname);
   //   LDBG_<<__FUNCTION__<<" -1 retrive prop:"<<propname<<" :"<<p->getJSONString()<< " full:"<<props.getJSONString();
      return p;
    }
    std::map<std::string, std::string>::iterator i =
        abstract2props.find(propname);
    if (i != abstract2props.end()) {
      if (props.hasKey(i->second) && props.isCDataWrapperValue(i->second)) {
        propname = i->second;
        chaos::common::data::CDWUniquePtr p=props.getCSDataValue(propname);
  //      LDBG_<<__FUNCTION__<<" -2 retrive prop:"<<propname<<" :"<<p->getJSONString()<< " full:"<<props.getJSONString();

        return p;
      }
    }
    return chaos::common::data::CDWUniquePtr();
  }
  chaos::common::data::CDWUniquePtr
  setProperty(const std::string &propname,
              const chaos::common::data::CDataWrapper &val, bool sync = false) {
    boost::mutex::scoped_lock ll (lock);
    std::string realpropname = propname;
    if(!val.hasKey("value")){
        throw chaos::CException(-10,propname+" missing required key 'value' in:"+val.getJSONString(),__FUNCTION__);
      }
    chaos::common::data::CDWUniquePtr prop = retriveProp(realpropname);
    if (prop.get()) {
     //DEBUG_CODE(( LDBG_ << __FUNCTION__ << "-"
     //       << "0 set property:" << propname<<" ["<<realpropname<<"]"<< ":" << prop->getJSONString()<<" full props:"<<props.getJSONString();
      
      chaos::common::data::CDWUniquePtr towrite = val.clone();

      ChaosStringVector sv;
      prop->getAllKey(sv);
      //if in the props there are keys that are not in the original one copy them (i.e. min,max)
      for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
        if (!towrite->hasKey(*i)) {
          prop->copyKeyTo(*i, *towrite.get());
        }
      }

      if (sync) {
        chaos::common::data::CDWUniquePtr ret =
            syncWrite(realpropname, towrite);
        if (ret.get()&& ret->hasKey("value")) {
         // ret->copyAllTo(props);
       /*  LDBG_ << __FUNCTION__ << "-"
            << "1 set property after sync: " << realpropname
            << " to :" << ret->getJSONString();*/
          replaceProperty(realpropname,*ret.get());
          //props.setValue(realpropname, ret.get());
        } else {
         /* LDBG_ << __FUNCTION__ << "-"
            << "2 set property sync returned empty: " << realpropname
            << " to :" << towrite->getJSONString();*/
          //towrite->copyAllTo(props);
          //props.setValue(realpropname, towrite.get());
          replaceProperty(realpropname,*towrite.get());
          
        }
      } else {
        //towrite->copyAllTo(props);
     /*   LDBG_ << __FUNCTION__ << "-"
            << "3 set property not sync: " << realpropname
            << " to :" << towrite->getJSONString();
       */   
        //props.setValue(realpropname, towrite.get());
        replaceProperty(realpropname,*towrite.get());
         
      }
      LDBG_ << __FUNCTION__ << "-"
            << "4 set property " << realpropname
            << " props:" << props.getJSONString()<<" input:"<<val.getJSONString();

      return retriveProp(realpropname);
    } else {
        LERR_ << __FUNCTION__ << "-"
            << "Property not exists, setting property \"" << realpropname
            << "\" props:" << props.getJSONString()<<" input:"<<val.getJSONString();

    }

    return chaos::common::data::CDWUniquePtr();
  }

  // set from cdatawrapper

  template <typename T>
  chaos::common::data::CDWUniquePtr
  createProperty(const std::string &propname, const T& value,
                 const std::string &pubname = "",
                 conversion_func_t getHandler = NULL,
                 conversion_func_t setHandler = NULL) {
    chaos::common::data::CDataWrapper p;
    p.append("value", value);
      if (setHandler) {
        prop2setHandler[propname] = setHandler;
      }
      if (getHandler) {
        prop2getHandler[propname] = getHandler;
      }
    if (!props.hasKey(propname)) {
      if (pubname.size() > 0) {
        p.append("pubname", pubname);
        abstract2props[pubname] = propname;
      }
      props.append(propname, p);
      syncWrite(propname);

    } else {
      return setProperty(propname, p);
    }
    LDBG_ << __FUNCTION__ << "-"
          << "1 create property:" << propname<<" :"<<p.getJSONString();

    return p.clone();
  }
  template <typename T>
  chaos::common::data::CDWUniquePtr
  createProperty(const std::string &propname, T value,  T min,  T max,  T incr,
                 const std::string &pubname = "",
                 conversion_func_t getHandler = NULL,
                 conversion_func_t setHandler = NULL) {
    chaos::common::data::CDataWrapper p;
    p.append("value", value);
    p.append("min", min);
    p.append("max", max);
    p.append("incr", incr);
    if (setHandler) {
        prop2setHandler[propname] = setHandler;
      }
      if (getHandler) {
        prop2getHandler[propname] = getHandler;
      }
    if (!props.hasKey(propname)) {
      if (pubname.size() > 0) {
        abstract2props[pubname] = propname;

        p.append("pubname", pubname);
      }
      props.append(propname, p);
      syncWrite(propname);

    } else {
      return setProperty(propname, p);
    }
    LDBG_ << __FUNCTION__ << "-"
          << "2 create property:" << propname<<":"<<p.getJSONString();
    return p.clone();
  }

  template <typename T>
  chaos::common::data::CDWUniquePtr
  setPropertyValue(const std::string &propname, const T &value,
                   bool sync = false) {
    chaos::common::data::CDataWrapper p;
    p.append("value", value);

    return setProperty(propname, p, sync);
  }

  chaos::common::data::CDWUniquePtr
  syncWrite(const std::string &propname,
            chaos::common::data::CDWUniquePtr &prop) {
    typename std::map<std::string, conversion_func_t>::iterator i =
        prop2setHandler.find(propname);
    if (i != prop2setHandler.end()) {
      return i->second((BC *)this, propname, *prop.get());
    }
    return prop->clone();
  }

  chaos::common::data::CDWUniquePtr
  syncWrite(const std::string &propname = "") {
    ChaosStringVector sv;
    props.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
      chaos::common::data::CDataWrapper p;
      typename std::map<std::string, conversion_func_t>::iterator j =
          prop2setHandler.find(*i);

      props.getCSDataValue(*i, p);
      if (j != prop2setHandler.end()) {
        if (propname == "") {
          chaos::common::data::CDWUniquePtr ret = j->second((BC *)this, *i, p);
          if (ret.get()) {
            setProperty(*i, *ret.get());
          }

        } else if (*i == propname) {
          chaos::common::data::CDWUniquePtr ret =
              j->second((BC *)this, propname, p);
          return setProperty(*i, *ret.get());
        }
      }
    }

    return props.clone();
  }

  chaos::common::data::CDWUniquePtr syncRead(const std::string &propname = "") {
    ChaosStringVector sv;
    props.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
      typename std::map<std::string, conversion_func_t>::iterator j =
          prop2getHandler.find(*i);
      chaos::common::data::CDataWrapper p;
      props.getCSDataValue(*i, p);
      if (j != prop2getHandler.end()) {
        if (propname == "") {
          chaos::common::data::CDWUniquePtr ret = j->second((BC *)this, *i, p);
          if (ret.get()) {
            setProperty(*i, *ret.get());
          }
        
        } else if (*i == propname) {
          chaos::common::data::CDWUniquePtr ret =
              j->second((BC *)this, propname, p);
          if (ret.get()) {
            return setProperty(*i, *ret.get());
          }
        }
      }
    }

    return props.clone();
  }
  chaos::common::data::CDWUniquePtr
  replaceProperty(const std::string &propname,
                  const chaos::common::data::CDataWrapper &p) {
    if (props.hasKey(propname)) {
     // props.setValue(propname, &p);
     props.replaceKey(propname,p);
     
    return p.clone();
    }
    chaos::common::data::CDWUniquePtr ret;
    return ret;
  }

  chaos::common::data::CDWUniquePtr
  setProperties(const chaos::common::data::CDataWrapper &p,bool sync=false) {
    ChaosStringVector sv;
    p.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
      if (p.isCDataWrapperValue(*i)) {
        chaos::common::data::CDataWrapper cd;
        p.getCSDataValue(*i, cd);
        if(cd.hasKey("value")){
          setProperty(*i, cd,sync);
        } else {
          LERR_<<__FUNCTION__<<" CDWrapper property:"<<*i<<" misses required key 'value'";
        }
      } else {
        chaos::common::data::CDataWrapper cd;
        p.copyKeyToNewKey(*i,"value",cd);
        setProperty(*i, cd,sync);

      }
    }
    return props.clone();
  }
  template <typename T>
  int getProperty(const std::string &propname, T &value, bool sync = false) {
    std::string realpropname = propname;
    chaos::common::data::CDWUniquePtr prop = retriveProp(realpropname);
    if (prop.get() == NULL) {
      return -1;
    }

    if (sync) {
      typename std::map<std::string, conversion_func_t>::iterator i =
          prop2getHandler.find(realpropname);
      if (i != prop2getHandler.end()) {
        prop = i->second((BC *)this, propname, *prop.get());
      }
    }

    if (prop->hasKey("value")) {
      value = props.getCSDataValue(realpropname)->getValue<T>("value");
      return 0;
    }

    return -2;
  }
   template <typename T>
  int getProperty(const std::string &propname, T &value, T &max, T &min,T &incr,bool sync = false) {
    std::string realpropname = propname;
    chaos::common::data::CDWUniquePtr prop = retriveProp(realpropname);
    if (prop.get() == NULL) {
      return -1;
    }

    if (sync) {
      typename std::map<std::string, conversion_func_t>::iterator i =
          prop2getHandler.find(realpropname);
      if (i != prop2getHandler.end()) {
        prop = i->second((BC *)this, propname, *prop.get());
      }
    }

   if (prop->hasKey("max")) {
      max = props.getCSDataValue(realpropname)->getValue<T>("max");
    }
  if (prop->hasKey("min")) {
      min = props.getCSDataValue(realpropname)->getValue<T>("min");
    }
    if (prop->hasKey("incr")) {
      incr = props.getCSDataValue(realpropname)->getValue<T>("incr");
    }
    if (prop->hasKey("value")) {
      value = props.getCSDataValue(realpropname)->getValue<T>("value");
      return 0;
    }
  
    return -2;
  }
};
} // namespace data
} // namespace misc
} // namespace common
#endif