#pragma once
#ifndef _EP_KVPSTORE_H
#define _EP_KVPSTORE_H

#include "ep/cpp/component/resource/resource.h"

namespace ep
{

SHARED_CLASS(KVPStore);

class KVPStore : public Resource
{
  EP_DECLARE_COMPONENT(KVPStore, Resource, EPKERNEL_PLUGINVERSION, "Metadata resource")
public:

  virtual size_t NumRecords() const { return 0; }

  virtual void Insert(Variant &&, Variant &&) {}
  virtual void Insert(const Variant &, Variant &&) {}
  virtual void Insert(Variant &&, const Variant &) {}
  virtual void Insert(const Variant &, const Variant &) {}

  virtual void Remove(const Variant &) {}

  virtual bool Exists(const Variant &) const { return false; }

  virtual Variant Get(const Variant &) const { return Variant(); }

  Variant operator[](const Variant &key) const
  {
    return Get(key);
  }

  // TODO: THIS IS ONLY HERE BECAUSE DESCRIPTOR CAN'T GET TO PROTECTED
  void InsertMethod(const Variant &key, const Variant &value) { Insert(key, value); }

protected:
  KVPStore(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_EXPLICIT("NumRecords", "Number of records in the metadata", EP_MAKE_GETTER(NumRecords), nullptr, nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD_EXPLICIT("Insert", InsertMethod, "Insert a record"),
      EP_MAKE_METHOD(Remove, "Remove a record"),
      EP_MAKE_METHOD(Exists, "Check if a record exists"),
      EP_MAKE_METHOD(Get, "Get a record"),
    };
  }
};

}

#endif // _EP_KVPSTORE_H
