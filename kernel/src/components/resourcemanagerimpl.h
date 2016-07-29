#pragma once
#ifndef EP_RESOURCEMANAGERIMPL_H
#define EP_RESOURCEMANAGERIMPL_H

#include "ep/cpp/component/resourcemanager.h"
#include "ep/cpp/internal/i/iresourcemanager.h"
#include "ep/cpp/component/component.h"

namespace ep {

class ResourceManagerImpl : public BaseImpl<ResourceManager, IResourceManager>
{
public:
  ResourceManagerImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  // Resource getter/setters
  size_t GetNumResources() const override final { return resources.size(); }
  void AddResource(ResourceRef res) override final { AddResourceArray(Slice<const ResourceRef>{ res }); }
  void AddResourceArray(Slice<const ResourceRef> resArray) override final;
  void RemoveResource(ResourceRef res) override final { RemoveResourceArray(Slice<const ResourceRef>{ res }); }
  void RemoveResourceArray(Slice<const ResourceRef> resArray) override final;

  void ClearResources() override final;
  ResourceRef GetResource(String key) const override final { return *resources.get(key); }
  Array<ResourceRef> GetResourceArray() const override final;

  template<typename CT>
  Array<ResourceRef> GetResourcesByType() const
  {
    return GetResourcesByType(&CT::descriptor);
  }
  Array<ResourceRef> GetResourcesByType(const ep::ComponentDesc *pBase) const override final;
  // TODO GetResourcesByPrefix with optional Type filter

  ResourceIterator begin() const override final { return ResourceIterator( resources.begin()); }
  ResourceIterator end() const override final { return ResourceIterator( resources.end()); }

  Variant::VarMap GetExtensions() const override final;
  // Resource loading/saving functions
  DataSourceRef LoadResourcesFromFile(Variant::VarMap initParams) override final;
  void SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams) override final;

  Variant Save() const override final { return pInstance->Super::save(); }

protected:
  AVLTree<String, ResourceRef> resources;
};

} //namespace ep
#endif // EP_RESOURCEMANAGERIMPL_H
