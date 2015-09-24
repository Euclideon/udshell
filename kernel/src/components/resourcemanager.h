#pragma once
#ifndef EP_RESOURCE_MANAGER_H
#define EP_RESOURCE_MANAGER_H

#include "component.h"

namespace ep
{
PROTOTYPE_COMPONENT(ResourceManager);
SHARED_CLASS(Resource);

class ResourceManager : public Component
{
public:
  EP_COMPONENT(ResourceManager);

  // Resource getter/setters
  size_t NumResources() const { return resources.Size(); }
  void AddResource(ResourceRef res);
  void RemoveResource(ResourceRef res);
  ResourceRef GetResource(epString key) const { return *resources.Get(key); }
  template<typename CT>
  epArray<ResourceRef> GetResourcesByType() const
  {
    return GetResourcesByType(&CT::descriptor);
  }
  epArray<ResourceRef> GetResourcesByType(const ComponentDesc *pBase) const;
  // TODO GetResourcesByPrefix with optional Type filter

  // Resource loading/saving functions
  void LoadResourcesFromFile(epInitParams initParams);
  void SaveResourcesToFile(epSlice<ResourceRef>, epInitParams initParams);

protected:
  ResourceManager(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams);
  ~ResourceManager();

  epAVLTree<epString, ResourceRef> resources;
};

} //namespace ep
#endif // EP_RESOURCE_MANAGER_H
