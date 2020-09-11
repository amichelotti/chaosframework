#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/global.h>
namespace chaos {
namespace common {
namespace data {
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

public:
  void reset() { props.reset(); }
  chaos::common::data::CDWUniquePtr getProperties(bool sync = false) {
    if (sync) {
      ChaosStringVector sv;
      props.getAllKey(sv);
      for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
        chaos::common::data::CDataWrapper cd;
        typename std::map<std::string, conversion_func_t>::iterator j =
            prop2getHandler.find(*i);
        if ((j != prop2getHandler.end())) {
          chaos::common::data::CDataWrapper cd;
          props.getCSDataValue(*i, cd);
          chaos::common::data::CDWUniquePtr toset =
              j->second((BC *)this, *i, cd);
          if (toset.get()) {
            props.setValue(*i, toset.get());
          }
        }
      }
    }
    return props.clone();
  }
  void importKeysAsProperties(chaos::common::data::CDataWrapper &p) {
    ChaosStringVector sv;
    p.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
        chaos::common::data::CDataWrapper cd;
        p.copyKeyToNewKey(*i,"value",cd);
        props.addCSDataValue(*i,cd);

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
       throw chaos::CException(-2,"missing required key 'value",__FUNCTION__);
     }
      props.append(
          propname,
          ((setHandler != NULL) ? *setHandler(propname, value).get() : value));

    } else {

      props.setValue(propname,
                     (const chaos::common::data::CDataWrapper *)&value);
    }
    return props.clone();
  }

  chaos::common::data::CDWUniquePtr retriveProp(std::string &propname) {
    if (props.hasKey(propname) && props.isCDataWrapperValue(propname)) {
      chaos::common::data::CDataWrapper p;
      props.getCSDataValue(propname, p);
      return p.clone();
    }
    std::map<std::string, std::string>::iterator i =
        abstract2props.find(propname);
    if (i != abstract2props.end()) {
      if (props.hasKey(i->second) && props.isCDataWrapperValue(i->second)) {
        propname = i->second;
        chaos::common::data::CDataWrapper p;
        props.getCSDataValue(i->second, p);
        return p.clone();
      }
    }
    return chaos::common::data::CDWUniquePtr();
  }
  chaos::common::data::CDWUniquePtr
  setProperty(const std::string &propname,
              chaos::common::data::CDataWrapper &val, bool sync = false) {
    std::string realpropname = propname;
    chaos::common::data::CDWUniquePtr prop = retriveProp(realpropname);
    if (prop.get()) {
      ChaosStringVector sv;
      prop->getAllKey(sv);
      chaos::common::data::CDWUniquePtr towrite = val.clone();

      for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
        if (!towrite->hasKey(*i)) {
          prop->copyKeyTo(*i, *towrite.get());
        }
      }

      if (sync) {
        chaos::common::data::CDWUniquePtr ret =
            syncWrite(realpropname, towrite);
        if (ret.get()) {
          props.setValue(realpropname, ret.get());
        }
      } else {
        props.setValue(realpropname, towrite.get());
      }
      LDBG_ << __FUNCTION__ << "-"
            << "set property " << realpropname
            << " props:" << props.getJSONString();

      return retriveProp(realpropname);
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
      
    } else {
      return setProperty(propname, p);
    }
    LDBG_ << __FUNCTION__ << "-"
          << "create property:" << p.getJSONString();

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
      

    } else {
      return setProperty(propname, p);
    }
    LDBG_ << __FUNCTION__ << "-"
          << "create property:" << p.getJSONString();
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
      props.setValue(propname, &p);
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
};
} // namespace data
} // namespace misc
} // namespace common