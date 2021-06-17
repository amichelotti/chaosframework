#include <chaos/common/data/CDataWrapper.h>
#include <chaos/common/global.h>
#ifndef __DATAPROPERTY__
#define __DATAPROPERTY__
namespace chaos {
namespace common {
namespace data {
#define PROPERTY_VALUE_KEY "VAL"
#define PROPERTY_VALUE_MAX_KEY "max"
#define PROPERTY_VALUE_MIN_KEY "min"
#define PROPERTY_VALUE_INC_KEY "inc"
#define PROPERTY_VALUE_PUB_KEY "pubname"
#define PROPERTY_VALUE_DESC_KEY "DESC"
#define PROPERTY_VALUE_UNIT_KEY "EGU"
#define PROPERTY_VALUE_COUNT_KEY "NELM"

#define PROPERTY_VALUE_IO_KEY "io"
#define PROPERTY_VALUE_PRECISION_KEY "PREC"
#define PROPERTY_VALUE_ALARM_LVL_KEY "alarm_lvl"

#define PROPERTY_VALUE_ERROR_LOW_KEY "LOLO"
#define PROPERTY_VALUE_ERROR_HI_KEY "HIHI"

#define PROPERTY_VALUE_WARN_LOW_KEY "LOW"
#define PROPERTY_VALUE_WARN_HI_KEY "HIGH"
#define PROPERTY_ERROR_SERVERITY_HI "HHSV" //Hihi Alarm Severity
#define PROPERTY_ERROR_SERVERITY_LO "LLSV" // Lolo Alarm Severity
#define PROPERTY_ERROR_WARNING_HI "HSV"  //High Alarm Severity
#define PROPERTY_ERROR_WARNING_LO "LSV"  //- Low Alarm Severity
#define PROPERTY_VALUE_MAXOPC_KEY "HOPR"
#define PROPERTY_VALUE_MINOPC_KEY "LOPR"

#define CREATE_DRV_INT_PROP(n, pub, var, min, max, inc, typ) \
  createProperty(                                            \
      n, var, (int32_t)min, (int32_t)max, (int32_t)inc, pub, [](AbstractDriver *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addInt32Value(PROPERTY_VALUE_KEY,((typ*)thi)->var);\
        return ret; }, [](AbstractDriver *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getInt32Value(PROPERTY_VALUE_KEY);\
          return p.clone(); });

#define CREATE_DRV_DOUBLE_PROP(n, pub, var, min, max, inc, typ) \
  createProperty(                                               \
      n, var, min, max, inc, pub, [](AbstractDriver *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addDoubleValue(PROPERTY_VALUE_KEY,((typ*)thi)->var);\
        return ret; }, [](AbstractDriver *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getDoubleValue(PROPERTY_VALUE_KEY);\
          return p.clone(); });

#define CREATE_DRV_BOOL_PROP(n, pub, var, typ) \
  createProperty(                              \
      n, var, pub, [](AbstractDriver *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addBoolValue(PROPERTY_VALUE_KEY,((typ*)thi)->var);\
        return ret; }, [](AbstractDriver *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getBoolValue(PROPERTY_VALUE_KEY);\
          return p.clone(); });

#define CREATE_DRV_STRING_PROP(n, pub, var, typ) \
  createProperty(                                \
      n, var, pub, [](AbstractDriver *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addStringValue(PROPERTY_VALUE_KEY,((typ*)thi)->var);\
        return ret; }, [](AbstractDriver *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getStringValue(PROPERTY_VALUE_KEY);\
          return p.clone(); });

#define CREATE_CU_INT_PROP(n, pub, var, min, max, inc, typ) \
  createProperty(                                           \
      n, var, (int32_t)min, (int32_t)max, (int32_t)inc, pub, [](AbstractControlUnit *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addInt32Value(PROPERTY_VALUE_KEY,((typ*)thi)->var);\
        return ret; }, [](AbstractControlUnit *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getInt32Value(PROPERTY_VALUE_KEY);\
          return p.clone(); });

#define CREATE_CU_DOUBLE_PROP(n, pub, var, min, max, inc, typ) \
  createProperty(                                              \
      n, var, min, max, inc, pub, [](AbstractControlUnit *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addDoubleValue(PROPERTY_VALUE_KEY,((typ*)thi)->var);\
        return ret; }, [](AbstractControlUnit *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getDoubleValue(PROPERTY_VALUE_KEY);\
          return p.clone(); });

#define CREATE_CU_BOOL_PROP(n, pub, var, typ) \
  createProperty(                             \
      n, var, pub, [](AbstractControlUnit *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addBoolValue(PROPERTY_VALUE_KEY,((typ*)thi)->var);\
        return ret; }, [](AbstractControlUnit *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getBoolValue(PROPERTY_VALUE_KEY);\
          return p.clone(); });

#define CREATE_CU_STRING_PROP(n, pub, var, typ) \
  createProperty(                               \
      n, var, pub, [](AbstractControlUnit *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr {\
        chaos::common::data::CDWUniquePtr ret(new chaos::common::data::CDataWrapper());\
        ret->addStringValue(PROPERTY_VALUE_KEY,((typ*)thi)->var);\
        return ret; }, [](AbstractControlUnit *thi, const std::string &name, const chaos::common::data::CDataWrapper &p) -> chaos::common::data::CDWUniquePtr { \
          ((typ*)thi)->var=p.getStringValue(PROPERTY_VALUE_KEY);\
          return p.clone(); });

#define FILLPROPERTYCD(cw, value, min, max, incr) \
  cw->append(PROPERTY_VALUE_KEY, value);          \
  cw->append(PROPERTY_VALUE_MIN_KEY, min);        \
  cw->append(PROPERTY_VALUE_MAX_KEY, max);        \
  cw->append(PROPERTY_VALUE_INC_KEY, incr);
/**
 * Class to store properties/variables that can be abstracted and private
 * typic example is in the use of camera properties that has to be normalized
 * for an abstracted use
 */
template <typename BC>
class Property {
  chaos::common::data::CDataWrapper props;
  typedef chaos::common::data::CDWUniquePtr (*conversion_func_t)(
      BC *,
      const std::string &name,
      const chaos::common::data::CDataWrapper &);
  std::map<std::string, std::string>       abstract2props;
  std::map<std::string, conversion_func_t> prop2setHandler;
  std::map<std::string, conversion_func_t> prop2getHandler;
  boost::recursive_mutex                   lock;

 public:
  void                              reset() { props.reset(); }
  chaos::common::data::CDWUniquePtr getProperties(bool sync = false) {
    boost::recursive_mutex::scoped_lock ll(lock);

    if (sync) {
      syncRead();
    }
    return props.clone();
  }
  void importKeysAsProperties(chaos::common::data::CDataWrapper &p, bool sync = false) {
    ChaosStringVector sv;
    p.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
      chaos::common::data::CDataWrapper cd;
      p.copyKeyToNewKey(*i, PROPERTY_VALUE_KEY, cd);
      setProperty(*i, cd, sync);
    }
  }

  void appendPropertiesTo(chaos::common::data::CDataWrapper &p) {
    ChaosStringVector sv;
    props.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
      chaos::common::data::CDataWrapper cd;
      if (props.isCDataWrapperValue(*i)) {
        props.getCSDataValue(*i, cd);
        if (cd.hasKey(PROPERTY_VALUE_KEY)) {
          p.copyKeyToNewKey(PROPERTY_VALUE_KEY, *i, p);
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
        if (cd.hasKey(PROPERTY_VALUE_PUB_KEY) && cd.isStringValue(PROPERTY_VALUE_PUB_KEY)) {
          std::string pub = cd.getStringValue(PROPERTY_VALUE_PUB_KEY);
          if ((pub.size() > 0) && cd.hasKey(PROPERTY_VALUE_KEY)) {
            cd.copyKeyToNewKey(PROPERTY_VALUE_KEY, pub, p);
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
      const std::string  propname,
      conversion_func_t  getHandler,
      conversion_func_t  setHandler,
      const std::string &pubname = "") {
    if (setHandler) {
      prop2setHandler[propname] = setHandler;
    }
    if (getHandler) {
      prop2getHandler[propname] = getHandler;
    } else {
      throw chaos::CException(-10, "Get handler of " + propname + " must be set", __FUNCTION__);
    }
    chaos::common::data::CDWUniquePtr ret = getHandler((BC *)this, propname, chaos::common::data::CDataWrapper());
    if (ret.get() == NULL) {
      LERR_ << "Cannot initialize property " + propname + " from get callback";
      return chaos::common::data::CDWUniquePtr();
    }
    if (!props.hasKey(propname)) {
      if (pubname.size() > 0) {
        ret->append(PROPERTY_VALUE_PUB_KEY, pubname);

        if ((getHandler != NULL) && (setHandler == NULL)) {
          ret->append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Output);
        } else if ((getHandler == NULL) && (setHandler != NULL)) {
          ret->append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Input);

        } else {
          ret->append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Bidirectional);
        }
        abstract2props[pubname] = propname;
      }
      if (ret.get()) {
        props.append(propname, *ret.get());
      }

    } else {
      setProperty(propname, *ret.get());
    }
    return props.clone();
  }

  chaos::common::data::CDWUniquePtr createProperty(
      const std::string &                propname,
      chaos::common::data::CDataWrapper &value,
      const std::string &                pubname    = "",
      conversion_func_t                  getHandler = NULL,
      conversion_func_t                  setHandler = NULL) {
    if (setHandler) {
      prop2setHandler[propname] = setHandler;
    }
    if (getHandler) {
      prop2getHandler[propname] = getHandler;
    }
    if (!props.hasKey(propname)) {
      chaos::common::data::CDWUniquePtr val;

      if (!value.hasKey(PROPERTY_VALUE_KEY)) {
        if (getHandler) {
          val = getHandler((BC *)this, propname, value);

        } else {
          throw chaos::CException(-2, propname + " missing required key'" PROPERTY_VALUE_KEY "'", __FUNCTION__);
        }

      } else {
        val = value.clone();
      }

      if (pubname.size() > 0) {
        val->append(PROPERTY_VALUE_PUB_KEY, pubname);
        abstract2props[pubname] = propname;
      }
      if (!val->hasKey(PROPERTY_VALUE_IO_KEY)) {
        if ((getHandler != NULL) && (setHandler == NULL)) {
          val->append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Output);
        } else if ((getHandler == NULL) && (setHandler != NULL)) {
          val->append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Input);

        } else {
          val->append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Bidirectional);
        }
      }
      props.append(propname, *val.get());

      syncWrite(propname);

    } else {
      setProperty(propname, value);
      // props.setValue(propname,
      //              (const chaos::common::data::CDataWrapper *)&value);
    }
    return props.clone();
  }

  chaos::common::data::CDWUniquePtr retriveProp(std::string &propname) {
    if (props.isCDataWrapperValue(propname)) {
      //   LDBG_<<__FUNCTION__<<" -0 retrive prop full before:"<<props.getJSONString();

      chaos::common::data::CDWUniquePtr p = props.getCSDataValue(propname);
      //   LDBG_<<__FUNCTION__<<" -1 retrive prop:"<<propname<<" :"<<p->getJSONString()<< " full:"<<props.getJSONString();
      return p;
    } else {
      LERR_ << propname << " doesnt match an object, trying public";
    }
    std::map<std::string, std::string>::iterator i =
        abstract2props.find(propname);
    if (i != abstract2props.end()) {
      if (props.isCDataWrapperValue(i->second)) {
        propname                            = i->second;
        chaos::common::data::CDWUniquePtr p = props.getCSDataValue(propname);
        //      LDBG_<<__FUNCTION__<<" -2 retrive prop:"<<propname<<" :"<<p->getJSONString()<< " full:"<<props.getJSONString();
        return p;
      } else {
        LERR_ << "public:" << propname << " doesnt match an object";
      }
    } else {
      LERR_ << propname << " not found in public";
    }
    return chaos::common::data::CDWUniquePtr();
  }
  /**
   * @brief Set an existing  Property object from key value string
   * 
   * @param propname property name
   * @param val property value
   * @param sync synchronize 
   * @return chaos::common::data::CDWUniquePtr 
   */
  chaos::common::data::CDWUniquePtr
  setProperty(const std::string &propname,
              const std::string &val,
              bool               sync = false) {
    std::string realpropname = propname;

    chaos::common::data::CDWUniquePtr prop = retriveProp(realpropname);
    if (prop.get()) {
      prop->setAsString(PROPERTY_VALUE_KEY, val);
      LDBG_ << __FUNCTION__ << "- Set property:" << realpropname << " string value:" << val << " full prop:" << prop->getJSONString();
      props.replaceKey(realpropname, *prop.get());
      if (sync) {
        syncWrite(realpropname, prop);
      }

      return prop;
    }
    return chaos::common::data::CDWUniquePtr();
  }

  chaos::common::data::CDWUniquePtr
  setProperty(const std::string &                      propname,
              const chaos::common::data::CDataWrapper &val,
              bool                                     sync = false) {
    std::string realpropname = propname;
 /*   if (!val.hasKey(PROPERTY_VALUE_KEY)) {
      throw chaos::CException(-10, propname + " missing required key 'value' in:" + val.getJSONString(), __FUNCTION__);
    }*/
    boost::recursive_mutex::scoped_lock ll(lock);

    chaos::common::data::CDWUniquePtr prop = retriveProp(realpropname);
    if (prop.get()) {
      //DEBUG_CODE(( LDBG_ << __FUNCTION__ << "-"
      //       << "0 set property:" << propname<<" ["<<realpropname<<"]"<< ":" << prop->getJSONString()<<" full props:"<<props.getJSONString();

      chaos::common::data::CDWUniquePtr towrite = val.clone();

      if (sync) {
        chaos::common::data::CDWUniquePtr ret =
            syncWrite(realpropname, towrite);
        if (ret.get() && ret->hasKey(PROPERTY_VALUE_KEY)) {
          // ret->copyAllTo(props);
          /*  LDBG_ << __FUNCTION__ << "-"
            << "1 set property after sync: " << realpropname
            << " to :" << ret->getJSONString();*/
          replaceProperty(realpropname, *ret.get());
          return retriveProp(realpropname);

          //props.setValue(realpropname, ret.get());
        }
      }
      ChaosStringVector sv;
      prop->getAllKey(sv);
      //if in the props there are keys that are not in the original one copy them (i.e. min,max)
      for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
        if (!towrite->hasKey(*i)) {
          prop->copyKeyTo(*i, *towrite.get());
        }
      }
      replaceProperty(realpropname, *towrite.get());
      return retriveProp(realpropname);
    }
    LERR_ << __FUNCTION__ << "-"
          << "Property not exists, setting property \"" << realpropname
          << "\" props:" << props.getJSONString() << " input:" << val.getJSONString();

    return chaos::common::data::CDWUniquePtr();
  }

  // set from cdatawrapper

  template <typename T>
  chaos::common::data::CDWUniquePtr
  createProperty(const std::string &propname, const T &value, const std::string &pubname = "", conversion_func_t getHandler = NULL, conversion_func_t setHandler = NULL) {
    chaos::common::data::CDataWrapper p;
    p.append(PROPERTY_VALUE_KEY, value);
    if ((getHandler != NULL) && (setHandler == NULL)) {
      p.append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Output);
    } else if ((getHandler == NULL) && (setHandler != NULL)) {
      p.append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Input);

    } else {
      p.append(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Bidirectional);
    }
    if (setHandler) {
      prop2setHandler[propname] = setHandler;
    }
    if (getHandler) {
      prop2getHandler[propname] = getHandler;
    }
    if (!props.hasKey(propname)) {
      if (pubname.size() > 0) {
        p.append(PROPERTY_VALUE_PUB_KEY, pubname);
        abstract2props[pubname] = propname;
      }
      props.append(propname, p);
      syncWrite(propname);

    } else {
      return setProperty(propname, p);
    }
    LDBG_ << __FUNCTION__ << "-"
          << "1 create property:" << propname << " :" << p.getJSONString();

    return p.clone();
  }
  template <typename T>
  chaos::common::data::CDWUniquePtr
  createProperty(const std::string &propname, T value, T min, T max, T incr, const std::string &pubname = "", conversion_func_t getHandler = NULL, conversion_func_t setHandler = NULL) {
    return createProperty(propname, value, min, max, incr, "", "NA", pubname, getHandler, setHandler);
  }
  template <typename T>
  chaos::common::data::CDWUniquePtr
  createProperty(const std::string &propname, T value, T min, T max, T incr, const std::string &desc, const std::string &unit, const std::string &pubname = "", conversion_func_t getHandler = NULL, conversion_func_t setHandler = NULL) {
    chaos::common::data::CDataWrapper p;

    p.append(PROPERTY_VALUE_KEY, value);
    p.append(PROPERTY_VALUE_MIN_KEY, min);
    p.append(PROPERTY_VALUE_MAX_KEY, max);
    p.append(PROPERTY_VALUE_INC_KEY, incr);
    p.addStringValue(PROPERTY_VALUE_DESC_KEY, desc);
    p.addStringValue(PROPERTY_VALUE_UNIT_KEY, unit);

    if ((getHandler != NULL) && (setHandler == NULL)) {
      p.addInt32Value(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Output);
    } else if ((getHandler == NULL) && (setHandler != NULL)) {
      p.addInt32Value(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Input);

    } else {
      p.addInt32Value(PROPERTY_VALUE_IO_KEY, (int32_t)chaos::DataType::Bidirectional);
    }

    if (setHandler) {
      prop2setHandler[propname] = setHandler;
    }
    if (getHandler) {
      prop2getHandler[propname] = getHandler;
    }
    if (!props.hasKey(propname)) {
      if (pubname.size() > 0) {
        abstract2props[pubname] = propname;

        p.addStringValue(PROPERTY_VALUE_PUB_KEY, pubname);
      }
      props.append(propname, p);
      syncWrite(propname);

    } else {
      return setProperty(propname, p);
    }
    LDBG_ << __FUNCTION__ << "-"
          << "2 create property:" << propname << ":" << p.getJSONString();
    return p.clone();
  }

  template <typename T>
  chaos::common::data::CDWUniquePtr
  setPropertyValue(const std::string &propname, const T &value, bool sync = false) {
    chaos::common::data::CDataWrapper p;
    p.append(PROPERTY_VALUE_KEY, value);

    return setProperty(propname, p, sync);
  }

  chaos::common::data::CDWUniquePtr
  syncWrite(const std::string &                propname,
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
      chaos::common::data::CDataWrapper                           p;
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
          if (ret.get()) {
            return setProperty(*i, *ret.get());
          }
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
            //            LDBG_<<"Setting property:\""<<*i<<"\"="<<ret->getJSONString();
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
  replaceProperty(const std::string &                      propname,
                  const chaos::common::data::CDataWrapper &p) {
    if (props.hasKey(propname)) {
      // props.setValue(propname, &p);
      props.replaceKey(propname, p);

      return p.clone();
    }
    chaos::common::data::CDWUniquePtr ret;
    return ret;
  }

  chaos::common::data::CDWUniquePtr
  setProperties(const chaos::common::data::CDataWrapper &p, bool sync = false) {
    ChaosStringVector sv;
    p.getAllKey(sv);
    for (ChaosStringVector::iterator i = sv.begin(); i != sv.end(); i++) {
      if (p.isCDataWrapperValue(*i)) {
        chaos::common::data::CDataWrapper cd;
        p.getCSDataValue(*i, cd);
        setProperty(*i, cd, sync);

       /* if (cd.hasKey(PROPERTY_VALUE_KEY)) {
          setProperty(*i, cd, sync);
        } else {
          LERR_ << __FUNCTION__ << " CDWrapper property:" << *i << " misses required key '"<<PROPERTY_VALUE_KEY<<"'";
        }*/
      } else {
        chaos::common::data::CDataWrapper cd;
        p.copyKeyToNewKey(*i, PROPERTY_VALUE_KEY, cd);
        setProperty(*i, cd, sync);
      }
    }
    return props.clone();
  }
  template <typename T>
  int getProperty(const std::string &propname, T &value, bool sync = false) {
    std::string                       realpropname = propname;
    chaos::common::data::CDWUniquePtr prop         = retriveProp(realpropname);
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

    if (prop.get() && prop->hasKey(PROPERTY_VALUE_KEY)) {
      chaos::common::data::CDWUniquePtr pp = props.getCSDataValue(realpropname);
      value                                = pp->getValue<T>(PROPERTY_VALUE_KEY);
      return 0;
    }

    return -2;
  }
  template <typename T>
  int getProperty(const std::string &propname, T &value, T &max, T &min, T &incr, bool sync = false) {
    std::string                       realpropname = propname;
    chaos::common::data::CDWUniquePtr prop         = retriveProp(realpropname);
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

    if (prop->hasKey(PROPERTY_VALUE_MAX_KEY)) {
      chaos::common::data::CDWUniquePtr pp = props.getCSDataValue(realpropname);
      max                                  = pp->getValue<T>(PROPERTY_VALUE_MAX_KEY);
    }
    if (prop->hasKey(PROPERTY_VALUE_MIN_KEY)) {
      chaos::common::data::CDWUniquePtr pp = props.getCSDataValue(realpropname);

      min = pp->getValue<T>(PROPERTY_VALUE_MIN_KEY);
    }
    if (prop->hasKey(PROPERTY_VALUE_INC_KEY)) {
      chaos::common::data::CDWUniquePtr pp = props.getCSDataValue(realpropname);

      incr = pp->getValue<T>(PROPERTY_VALUE_INC_KEY);
    }
    if (prop->hasKey(PROPERTY_VALUE_KEY)) {
      chaos::common::data::CDWUniquePtr pp = props.getCSDataValue(realpropname);

      value = pp->getValue<T>(PROPERTY_VALUE_KEY);
      return 0;
    }

    return -2;
  }
};
}  // namespace data
}  // namespace common
}  // namespace chaos
#endif