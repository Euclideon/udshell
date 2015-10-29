#pragma once
#ifndef EP_RESOURCE_MANAGER_H
#define EP_RESOURCE_MANAGER_H

#include "component.h"

namespace ep
{
PROTOTYPE_COMPONENT(ResourceManager);
SHARED_CLASS(Resource);
SHARED_CLASS(DataSource);

class ResourceManager : public Component
{
public:
  EP_COMPONENT(ResourceManager);

  // Resource getter/setters
  size_t NumResources() const { return resources.Size(); }
  void AddResource(ResourceRef res);
  void RemoveResource(ResourceRef res);
  ResourceRef GetResource(String key) const { return *resources.Get(key); }
  template<typename CT>
  Array<ResourceRef> GetResourcesByType() const
  {
    return GetResourcesByType(&CT::descriptor);
  }
  Array<ResourceRef> GetResourcesByType(const ComponentDesc *pBase) const;
  // TODO GetResourcesByPrefix with optional Type filter

  // Resource loading/saving functions
  DataSourceRef LoadResourcesFromFile(InitParams initParams);
  void SaveResourcesToFile(Slice<ResourceRef>, InitParams initParams);

protected:
  ResourceManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  ~ResourceManager();

  AVLTree<String, ResourceRef> resources;
};

} //namespace ep
#endif // EP_RESOURCE_MANAGER_H
