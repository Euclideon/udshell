#pragma once
#ifndef _EP_KVPSTORE_H
#define _EP_KVPSTORE_H

#include "components/resources/resource.h"

namespace ep
{

SHARED_CLASS(KVPStore);

class KVPStore : public Resource
{
public:
  EP_COMPONENT(KVPStore);

  virtual size_t NumRecords() const { return 0; }

  virtual void Insert(epVariant &&, epVariant &&) {}
  virtual void Insert(const epVariant &, epVariant &&) {}
  virtual void Insert(epVariant &&, const epVariant &) {}
  virtual void Insert(const epVariant &, const epVariant &) {}

  virtual void Remove(const epVariant &) {}

  virtual bool Exists(const epVariant &) const { return false; }

  virtual epVariant Get(const epVariant &) const { return epVariant(); }

  epVariant operator[](const epVariant &key) const
  {
    return Get(key);
  }

  // TODO: THIS IS ONLY HERE BECAUSE DESCRIPTOR CAN'T GET TO PROTECTED
  void InsertMethod(const epVariant &key, const epVariant &value) { Insert(key, value); }

protected:
  KVPStore(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
};

}

#endif // _EP_KVPSTORE_H
