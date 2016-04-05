#pragma once
#ifndef _EP_KVPSTORE_H
#define _EP_KVPSTORE_H

#include "ep/cpp/component/resource/resource.h"

namespace ep {

SHARED_CLASS(KVPStore);

class KVPStore : public Resource
{
  EP_DECLARE_COMPONENT(ep, KVPStore, Resource, EPKERNEL_PLUGINVERSION, "KVPStore resource", 0)

public:
  virtual size_t NumRecords() const { return 0; }

  virtual void Insert(Variant &&key, Variant &&value) {}
  virtual void Insert(const Variant &key, Variant &&value) {}
  virtual void Insert(Variant &&key, const Variant &value) {}
  virtual void Insert(const Variant &key, const Variant &value) {}

  virtual void Remove(const Variant &key) {}
  virtual bool Exists(const Variant &key) const { return false; }
  virtual Variant Get(const Variant &key) const { return Variant(); }

  Variant operator[](const Variant &key) const { return Get(key); }

protected:
  KVPStore(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {}

  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;

private:
  void InsertMethod(const Variant &key, const Variant &value) { Insert(key, value); }
};

}

#endif // _EP_KVPSTORE_H
