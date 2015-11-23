#pragma once
#ifndef _EP_KVPSTORE_H
#define _EP_KVPSTORE_H

#include "components/resources/resource.h"

namespace kernel
{

SHARED_CLASS(KVPStore);

class KVPStore : public Resource
{
public:
  EP_COMPONENT(KVPStore);

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
  KVPStore(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
};

}

#endif // _EP_KVPSTORE_H
