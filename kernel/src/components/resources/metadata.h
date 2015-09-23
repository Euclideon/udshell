#pragma once
#ifndef _UD_METADATA_H
#define _UD_METADATA_H

#include "components/resources/kvpstore.h"
#include "ep/epavltree.h"

namespace ud
{

SHARED_CLASS(Metadata);

class Metadata : public KVPStore
{
public:
  UD_COMPONENT(Metadata);

  size_t NumRecords() const override
  {
    return metadata.Size();
  }

  void Insert(udVariant &&key, udVariant &&value) override           { metadata.Insert(std::move(key), std::move(value)); }
  void Insert(const udVariant &key, udVariant &&value) override      { metadata.Insert(key, std::move(value)); }
  void Insert(udVariant &&key, const udVariant &value) override      { metadata.Insert(std::move(key), value); }
  void Insert(const udVariant &key, const udVariant &value) override { metadata.Insert(key, value); }

  void Remove(const udVariant &key) override
  {
    metadata.Remove(key);
  }

  bool Exists(const udVariant &key) const override
  {
    return !!metadata.Get(key);
  }

  udVariant Get(const udVariant &key) const override
  {
    udVariant *v = const_cast<udVariant*>(metadata.Get(key));
    return v ? *v : udVariant();
  }

protected:
  Metadata(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : KVPStore(pType, pKernel, uid, initParams) {}

  struct VariantCompare {
    UDFORCE_INLINE ptrdiff_t operator()(const udVariant &a, const udVariant &b)
    {
      return a.compare(b);
    }
  };

  udAVLTree<udVariant, udVariant, VariantCompare> metadata;
};

}

#endif // _UD_METADATA_H
