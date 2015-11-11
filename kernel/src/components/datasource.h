#pragma once
#if !defined(_EP_DATASOURCE_H)
#define _EP_DATASOURCE_H

#include "component.h"
#include "components/stream.h"
#include "resources/resource.h"

namespace ep
{

PROTOTYPE_COMPONENT(DataSource);

class DataSource : public Component
{
public:
  EP_COMPONENT(Component);

  enum class Flags : size_t
  {
    ReadOnly = 1<<0,
    WriteOnly = 1<<1,
    FlushImmediately = 1<<2,
    DeferredLoad = 1<<3,
  };

  virtual Slice<const String> GetFileExtensions() const { return nullptr; }

  virtual epResult Flush()
  {
    return epR_Success;
  }

  size_t GetNumResources() const
  {
    return resources.Size();
  }
  String GetResourceName(size_t index) const
  {
    size_t i = 0;
    for (auto iter = resources.begin(); iter != resources.end(); ++iter)
    {
      if (i == index)
        return iter.Key();
      ++i;
    }
    return nullptr;
  }

  ResourceRef GetResource(size_t index) const
  {
    size_t i = 0;
    for (auto &r: resources)
    {
      if (i == index)
        return r;
      ++i;
    }
    return nullptr;
  }
  ResourceRef GetResource(String _name) const
  {
    ResourceRef *r = const_cast<ResourceRef*>(resources.Get(_name));
    if (r)
      return *r;
    return nullptr;
  }

  template<typename T>
  SharedPtr<T> GetResourceAs(size_t index) const
  {
    return component_cast<T>(GetResource(index));
  }
  template<typename T>
  SharedPtr<T> GetResourceAs(String name) const
  {
    return component_cast<T>(GetResource(name));
  }

  void SetResource(String _name, const ResourceRef &spResource)
  {
    resources.Insert(_name, spResource);
  }

  size_t CountResources(String prefix)
  {
    // TODO: ...
    return 0;
  }

  template<typename ResType = Resource>
  ResourceRef Get(size_t resourceIndex) const
  {
    return shared_pointer_cast<ResType>(GetResource(ResType::Name(), resourceIndex));
  }

  // HACK: this should be private!!
  ResourceRef GetResourceByVariant(Variant index) const
  {
    if (index.is(Variant::Type::String))
      return GetResource(index.asString());
    else
      return GetResource((size_t)index.asInt());
  }

  String GetURL() const { return url; }
  void SetURL(String _url) { url = _url; }

protected:
  DataSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);

  StreamRef OpenStream(const Variant &source);

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

#endif // _EP_DATASOURCE_H
