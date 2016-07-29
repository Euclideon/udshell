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
  virtual size_t numRecords() const { return 0; }

  virtual void insert(Variant &&key, Variant &&value) {}
  virtual void insert(const Variant &key, Variant &&value) {}
  virtual void insert(Variant &&key, const Variant &value) {}
  virtual void insert(const Variant &key, const Variant &value) {}

  virtual void remove(const Variant &key) {}
  virtual bool exists(const Variant &key) const { return false; }
  virtual Variant get(const Variant &key) const { return Variant(); }

  Variant operator[](const Variant &key) const { return get(key); }

protected:
  KVPStore(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {}

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;

private:
  void insertMethod(const Variant &key, const Variant &value) { insert(key, value); }
};

}

#endif // _EP_KVPSTORE_H
