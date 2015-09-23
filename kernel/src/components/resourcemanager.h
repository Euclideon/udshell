#pragma once
#ifndef UD_RESOURCE_MANAGER_H
#define UD_RESOURCE_MANAGER_H

#include "component.h"

namespace ud
{
PROTOTYPE_COMPONENT(ResourceManager);
SHARED_CLASS(Resource);

class ResourceManager : public Component
{
public:
  UD_COMPONENT(ResourceManager);

  // Resource getter/setters
  size_t NumResources() const { return resources.Size(); }
  void AddResource(ResourceRef res);
  void RemoveResource(ResourceRef res);
  ResourceRef GetResource(udString key) const { return *resources.Get(key); }
  template<typename CT>
  udFixedSlice<ResourceRef> GetResourcesByType() const
  {
    return GetResourcesByType(&CT::descriptor);
  }
  udFixedSlice<ResourceRef> GetResourcesByType(const ComponentDesc *pBase) const;
  // TODO GetResourcesByPrefix with optional Type filter

  // Resource loading/saving functions
  void LoadResourcesFromFile(udInitParams initParams);
  void SaveResourcesToFile(udSlice<ResourceRef>, udInitParams initParams);

protected:
  ResourceManager(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  ~ResourceManager();

  udAVLTree<udString, ResourceRef> resources;
};

} //namespace ud
#endif // UD_RESOURCE_MANAGER_H
