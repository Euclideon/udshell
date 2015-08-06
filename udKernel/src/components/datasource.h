#pragma once
#if !defined(_UD_DATASOURCE_H)
#define _UD_DATASOURCE_H

#include "component.h"
#include "components/stream.h"
#include "resources/resource.h"

namespace ud
{

PROTOTYPE_COMPONENT(DataSource);

class DataSource : public Component
{
public:
  UD_COMPONENT(Component);

  enum class Flags : size_t
  {
    ReadOnly = 1<<0,
    WriteOnly = 1<<1,
    FlushImmediately = 1<<2,
    DeferredLoad = 1<<3,
  };

  virtual udSlice<const udString> GetFileExtensions() const { return nullptr; }

  virtual udResult Flush()
  {
    return udR_Success;
  }

  size_t GetNumResources() const
  {
    return resources.Size();
  }
  udString GetResourceName(size_t index) const
  {
    // index map... somehow
    return nullptr;
  }
  ResourceRef GetResource(size_t index) const
  {
    // index map... somehow
    return nullptr;
  }
  ResourceRef GetResource(udString name) const
  {
    ResourceRef *r = const_cast<ResourceRef*>(resources.Get(name));
    if (r)
      return *r;
    return nullptr;
  }

  void SetResource(udString name, const ResourceRef &spResource)
  {
    resources.Insert(name, spResource);
  }

  size_t CountResources(udString prefix)
  {
    // TODO: ...
    return 0;
  }

  template<typename ResType = Resource>
  ResourceRef Get(size_t resourceIndex) const
  {
    return static_pointer_cast<ResType>(GetResource(ResType::Name(), resourceIndex));
  }

  // HACK: this should be private!!
  ResourceRef GetResourceByVariant(udVariant index) const
  {
    if (index.is(udVariant::Type::String))
      return GetResource(index.asString());
    else
      return GetResource(index.asInt());
  }

protected:
  DataSource(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);

  virtual void Create(StreamRef spSource) {}

  struct StringCompare {
    UDFORCE_INLINE ptrdiff_t operator()(udString a, udString b)
    {
      return a.cmp(b);
    }
  };

  udAVLTree<udString, ResourceRef, StringCompare> resources;
  Flags flags;
};

}

#endif // _UD_DATASOURCE_H
