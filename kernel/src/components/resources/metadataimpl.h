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
    return metadata.size();
  }

  void InsertRR(Variant &&key, Variant &&value) override final           { metadata.replace(std::move(key), std::move(value)); }
  void InsertLR(const Variant &key, Variant &&value) override final      { metadata.replace(key, std::move(value)); }
  void InsertRL(Variant &&key, const Variant &value) override final      { metadata.replace(std::move(key), value); }
  void InsertLL(const Variant &key, const Variant &value) override final { metadata.replace(key, value); }

  void Remove(const Variant &key) override final
  {
    metadata.remove(key);
  }

  bool Exists(const Variant &key) const override final
  {
    return !!metadata.get(key);
  }

  Variant Get(const Variant &key) const override final
  {
    Variant *v = const_cast<Variant*>(metadata.get(key));
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

inline Array<const PropertyInfo> Metadata::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("numRecords", NumRecords, "Number of records in the metadata", nullptr, 0),
  };
}

inline Array<const MethodInfo> Metadata::getMethods() const
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
