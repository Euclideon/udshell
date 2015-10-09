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

  virtual epSlice<const epString> GetFileExtensions() const { return nullptr; }

  virtual udResult Flush()
  {
    return udR_Success;
  }

  size_t GetNumResources() const
  {
    return resources.Size();
  }
  epString GetResourceName(size_t index) const
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
  ResourceRef GetResource(epString name) const
  {
    ResourceRef *r = const_cast<ResourceRef*>(resources.Get(name));
    if (r)
      return *r;
    return nullptr;
  }

  template<typename T>
  epSharedPtr<T> GetResourceAs(size_t index) const
  {
    return component_cast<T>(GetResource(index));
  }
  template<typename T>
  epSharedPtr<T> GetResourceAs(epString name) const
  {
    return component_cast<T>(GetResource(name));
  }

  void SetResource(epString name, const ResourceRef &spResource)
  {
    resources.Insert(name, spResource);
  }

  size_t CountResources(epString prefix)
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
  ResourceRef GetResourceByVariant(epVariant index) const
  {
    if (index.is(epVariant::Type::String))
      return GetResource(index.asString());
    else
      return GetResource((size_t)index.asInt());
  }

protected:
  DataSource(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);

  StreamRef OpenStream(const epVariant &source);

  struct StringCompare {
    EPALWAYS_INLINE ptrdiff_t operator()(epString a, epString b)
    {
      return a.cmp(b);
    }
  };

  epAVLTree<epSharedString, ResourceRef> resources;
  Flags flags;
};

}

#endif // _EP_DATASOURCE_H
