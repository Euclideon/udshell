#pragma once
#ifndef _UD_METADATA_H
#define _UD_METADATA_H

#include "components/resources/resource.h"
#include "util/udavltree.h"

namespace ud
{

SHARED_CLASS(Metadata);

class Metadata : public Resource
{
public:
  UD_COMPONENT(Metadata);

  size_t NumRecords() const
  {
    return metadata.Size();
  }

  void Insert(udVariant &&key, udVariant &&value)           { metadata.Insert(std::move(key), std::move(value)); }
  void Insert(const udVariant &key, udVariant &&value)      { metadata.Insert(key, std::move(value)); }
  void Insert(udVariant &&key, const udVariant &value)      { metadata.Insert(std::move(key), value); }
  void Insert(const udVariant &key, const udVariant &value) { metadata.Insert(key, value); }

  void Remove(const udVariant &key)
  {
    metadata.Remove(key);
  }

  bool Exists(const udVariant &key) const
  {
    return !!metadata.Get(key);
  }

  udVariant Get(const udVariant &key) const
  {
    const udVariant *v = metadata.Get(key);
    return v ? *v : udVariant();
  }

  udVariant operator[](const udVariant &key) const
  {
    const udVariant *v = metadata.Get(key);
    return v ? *v : udVariant();
  }

  // TODO: THIS IS ONLY HERE BECAUSE DESCRIPTOR CAN'T GET TO PROTECTED
  void InsertMethod(const udVariant &key, const udVariant &value) { metadata.Insert(key, value); }

protected:
  Metadata(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

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
