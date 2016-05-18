#pragma once
#ifndef _EP_METADATAIMPL_H
#define _EP_METADATAIMPL_H

#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/avltree.h"

namespace ep {

class MetadataImpl : public BaseImpl<Metadata, IMetadata>
{
public:
  MetadataImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  size_t NumRecords() const override final
  {
    return metadata.Size();
  }

  void Insert(Variant &&key, Variant &&value) override final           { metadata.Replace(std::move(key), std::move(value)); }
  void Insert(const Variant &key, Variant &&value) override final      { metadata.Replace(key, std::move(value)); }
  void Insert(Variant &&key, const Variant &value) override final      { metadata.Replace(std::move(key), value); }
  void Insert(const Variant &key, const Variant &value) override final { metadata.Replace(key, value); }

  void Remove(const Variant &key) override final
  {
    metadata.Remove(key);
  }

  bool Exists(const Variant &key) const override final
  {
    return !!metadata.Get(key);
  }

  Variant Get(const Variant &key) const override final
  {
    Variant *v = const_cast<Variant*>(metadata.Get(key));
    return v ? *v : Variant();
  }

protected:
  struct VariantCompare {
    epforceinline ptrdiff_t operator()(const Variant &a, const Variant &b)
    {
      return a.compare(b);
    }
  };

  AVLTree<Variant, Variant, VariantCompare> metadata;
};

inline Array<const PropertyInfo> Metadata::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_EXPLICIT("NumRecords", "Number of records in the metadata", EP_MAKE_GETTER(NumRecords), nullptr, nullptr, 0),
  };
}

inline Array<const MethodInfo> Metadata::GetMethods() const
{
  return{
    EP_MAKE_METHOD_EXPLICIT("Insert", InsertMethod, "Insert a record"),
    EP_MAKE_METHOD(Remove, "Remove a record"),
    EP_MAKE_METHOD(Exists, "Check if a record exists"),
    EP_MAKE_METHOD(Get, "Get a record"),
  };
}

}

#endif // _EP_METADATAIMPL_H
