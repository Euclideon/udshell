#pragma once
#ifndef _UD_KVPSTORE_H
#define _UD_KVPSTORE_H

#include "components/resources/resource.h"

namespace ud
{

SHARED_CLASS(KVPStore);

class KVPStore : public Resource
{
public:
  UD_COMPONENT(KVPStore);

  virtual size_t NumRecords() const { return 0; }

  virtual void Insert(udVariant &&, udVariant &&) {}
  virtual void Insert(const udVariant &, udVariant &&) {}
  virtual void Insert(udVariant &&, const udVariant &) {}
  virtual void Insert(const udVariant &, const udVariant &) {}

  virtual void Remove(const udVariant &) {}

  virtual bool Exists(const udVariant &) const { return false; }

  virtual udVariant Get(const udVariant &) const { return udVariant(); }

  udVariant operator[](const udVariant &key) const
  {
    return Get(key);
  }

  // TODO: THIS IS ONLY HERE BECAUSE DESCRIPTOR CAN'T GET TO PROTECTED
  void InsertMethod(const udVariant &key, const udVariant &value) { Insert(key, value); }

protected:
  KVPStore(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
};

}

#endif // _UD_KVPSTORE_H
