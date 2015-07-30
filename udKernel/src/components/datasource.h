#pragma once
#if !defined(_UD_DATASOURCE_H)
#define _UD_DATASOURCE_H

#include "component.h"
#include "resources/resource.h"

namespace ud
{

class DataSource : Component
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

  udResult Flush();

  size_t GetNumResources() const { return resources.length; }
  udSlice<ResourceRef> GetResources() const { return resources; }

  ResourceRef GetResource(size_t resourceIndex) const { return resources[resourceIndex]; }
  int GetResourceType(size_t resourceIndex) const { return resources[resourceIndex]->Type(); }

  template<typename ResType = Resource>
  ResourceRef Get(size_t resourceIndex) const
  {
    return static_pointer_cast<ResType>(resources[resourceIndex]);
  }

protected:
  DataSource(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams) {}

  udFixedSlice<ResourceRef, 1> resources;
  Flags flags;
};

}

#endif // _UD_DATASOURCE_H
