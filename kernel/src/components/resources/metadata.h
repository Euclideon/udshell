#pragma once
#ifndef _EP_METADATA_H
#define _EP_METADATA_H

#include "components/resources/kvpstore.h"
#include "ep/cpp/avltree.h"

namespace ep
{

SHARED_CLASS(Metadata);

class Metadata : public KVPStore
{
  EP_DECLARE_COMPONENT(Metadata, KVPStore, EPKERNEL_PLUGINVERSION, "Metadata resource", 0)
public:

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
  Metadata(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : KVPStore(pType, pKernel, uid, initParams) {}

  struct VariantCompare {
    epforceinline ptrdiff_t operator()(const Variant &a, const Variant &b)
    {
      return a.compare(b);
    }
  };

  AVLTree<Variant, Variant, VariantCompare> metadata;

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

private:
  void InsertMethod(const Variant &key, const Variant &value) { Metadata::Insert(key, value); }
};

}

#endif // _EP_METADATA_H
