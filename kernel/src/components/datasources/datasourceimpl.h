#pragma once
#ifndef EPDATASOURCEIMPL_H
#define EPDATASOURCEIMPL_H

#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/internal/i/idatasource.h"

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/stream.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/resource/metadata.h"

namespace ep {

class DataSourceImpl : public BaseImpl<DataSource, IDataSource>
{
public:
  DataSourceImpl(Component *pInstance, Variant::VarMap initParams);

  enum class Flags : size_t
  {
    ReadOnly = 1<<0,
    WriteOnly = 1<<1,
    FlushImmediately = 1<<2,
    DeferredLoad = 1<<3,
  };

  Slice<const String> GetFileExtensions() const override final { return nullptr; }

  void Flush() override final
  {
  }

  size_t GetNumResources() const override final
  {
    return resources.size();
  }
  String GetResourceName(size_t index) const override final
  {
    size_t i = 0;
    for (auto iter : resources)
    {
      if (i == index)
        return iter.key;
      ++i;
    }
    return nullptr;
  }

  ResourceRef GetResourceFromIndex(size_t index) const override final
  {
    size_t i = 0;
    for (auto r: resources)
    {
      if (i == index)
        return r.value;
      ++i;
    }
    return nullptr;
  }

  ResourceRef GetResource(String _name) const override final
  {
    ResourceRef *r = const_cast<ResourceRef*>(resources.get(_name));
    if (r)
      return *r;
    return nullptr;
  }

  void SetResource(String _name, const ResourceRef &spResource) override final
  {
    MetadataRef spMeta = spResource->getMetadata();

    Variant resUrl = spMeta->get("url");
    if(!resUrl.is(Variant::Type::String))
      spMeta->insert("url", GetURL());

    resources.replace(_name, spResource);
  }

  size_t CountResources(String filterPattern) const override final
  {
    // TODO: ...
    return 0;
  }

  String GetURL() const override final { return url; }
  void SetURL(String _url) override final { url = _url; }

  Variant Save() const override final { return pInstance->Super::save(); }

protected:
  StreamRef OpenStream(const Variant &source) override final;

  struct StringCompare {
    epforceinline ptrdiff_t operator()(String a, String b)
    {
      return a.cmp(b);
    }
  };

  SharedString url;

  AVLTree<SharedString, ResourceRef> resources;
  Flags flags;
};

}

#endif // _EP_DATASOURCEIMPL_H
