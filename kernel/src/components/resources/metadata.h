#pragma once
#ifndef _EP_METADATA_H
#define _EP_METADATA_H

#include "components/resources/kvpstore.h"
#include "ep/cpp/avltree.h"

namespace kernel
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

  void Insert(Variant &&key, Variant &&value) override           { metadata.Insert(std::move(key), std::move(value)); }
  void Insert(const Variant &key, Variant &&value) override      { metadata.Insert(key, std::move(value)); }
  void Insert(Variant &&key, const Variant &value) override      { metadata.Insert(std::move(key), value); }
  void Insert(const Variant &key, const Variant &value) override { metadata.Insert(key, value); }

  void Remove(const Variant &key) override
  {
    metadata.Remove(key);
  }

  bool Exists(const Variant &key) const override
  {
    return !!metadata.Get(key);
  }

  Variant Get(const Variant &key) const override
  {
    Variant *v = const_cast<Variant*>(metadata.Get(key));
    return v ? *v : Variant();
  }

protected:
  Metadata(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : KVPStore(pType, pKernel, uid, initParams) {}

  struct VariantCompare {
    epforceinline ptrdiff_t operator()(const Variant &a, const Variant &b)
    {
      return a.compare(b);
    }
  };

  AVLTree<Variant, Variant, VariantCompare> metadata;
};

}

#endif // _EP_METADATA_H
