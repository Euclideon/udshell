#pragma once
#ifndef EP_RESOURCE_MANAGER_H
#define EP_RESOURCE_MANAGER_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iresourcemanager.h"
#include "ep/cpp/component/datasource/datasource.h"

namespace ep
{
SHARED_CLASS(ResourceManager);
SHARED_CLASS(Resource);

class ResourceIterator
{
public:
  ResourceIterator(AVLTree<String, ResourceRef>::Iterator iter)
    : iter(iter)
  {}

  ResourceIterator &operator++()
  {
    ++iter;
    return *this;
  }

  bool operator!=(ResourceIterator rhs) { return iter != rhs.iter; }

  ResourceRef operator*() const { return (*iter).value; }

  AVLTree<String, ResourceRef>::Iterator iter;
};

class ResourceManager : public Component, public IResourceManager
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ResourceManager, IResourceManager, Component, EPKERNEL_PLUGINVERSION, "ResourceManager desc...")

public:
  // Resource getter/setters
  size_t GetNumResources() const override final { return pImpl->GetNumResources(); }
  void AddResource(ResourceRef res) override final { pImpl->AddResource(res); }
  void RemoveResource(ResourceRef res) override final { return pImpl->RemoveResource(res); }
  ResourceRef GetResource(String key) const override final { return pImpl->GetResource(key); }
  template<typename CT>
  Array<ResourceRef> GetResourcesByType() const
  {
    return GetResourcesByType(&CT::descriptor);
  }
  Array<ResourceRef> GetResourcesByType(const ep::ComponentDesc *pBase) const override final { return pImpl->GetResourcesByType(pBase); }

  ResourceIterator begin() const { return pImpl->begin(); }
  ResourceIterator end() const { return pImpl->end(); }

  // Resource loading/saving functions
  DataSourceRef LoadResourcesFromFile(Variant::VarMap initParams) override final { return pImpl->LoadResourcesFromFile(initParams); }
  void SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams) override final { pImpl->SaveResourcesToFile(spDataSource, initParams); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  ResourceManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(NumResources, "The number of Resources in the ResourceManager", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(AddResource, "Add a Resource to the ResourceManager"),
      EP_MAKE_METHOD(RemoveResource, "Remove the specified Resource"),
      EP_MAKE_METHOD(GetResource, "Get Resource by UID"),
      EP_MAKE_METHOD(LoadResourcesFromFile, "Create a DataSource containing Resources from the file specified by the given File InitParams"),
      EP_MAKE_METHOD(SaveResourcesToFile, "Save Resources from the given DataSource to a file specified by the given File InitParams"),
    };
  }
};

} //namespace ep
#endif // EP_RESOURCE_MANAGER_H
