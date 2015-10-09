#pragma once
#ifndef _EP_METADATA_H
#define _EP_METADATA_H

#include "components/resources/kvpstore.h"
#include "ep/epavltree.h"

namespace ep
{

SHARED_CLASS(Metadata);

class Metadata : public KVPStore
{
public:
  EP_COMPONENT(Metadata);

  size_t NumRecords() const override
  {
    return metadata.Size();
  }

  void Insert(epVariant &&key, epVariant &&value) override           { metadata.Insert(std::move(key), std::move(value)); }
  void Insert(const epVariant &key, epVariant &&value) override      { metadata.Insert(key, std::move(value)); }
  void Insert(epVariant &&key, const epVariant &value) override      { metadata.Insert(std::move(key), value); }
  void Insert(const epVariant &key, const epVariant &value) override { metadata.Insert(key, value); }

  void Remove(const epVariant &key) override
  {
    metadata.Remove(key);
  }

  bool Exists(const epVariant &key) const override
  {
    return !!metadata.Get(key);
  }

  epVariant Get(const epVariant &key) const override
  {
    epVariant *v = const_cast<epVariant*>(metadata.Get(key));
    return v ? *v : epVariant();
  }

protected:
  Metadata(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : KVPStore(pType, pKernel, uid, initParams) {}

  struct VariantCompare {
    epforceinline ptrdiff_t operator()(const epVariant &a, const epVariant &b)
    {
      return a.compare(b);
    }
  };

  epAVLTree<epVariant, epVariant, VariantCompare> metadata;
};

}

#endif // _EP_METADATA_H
