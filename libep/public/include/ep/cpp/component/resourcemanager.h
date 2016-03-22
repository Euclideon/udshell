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

// TODO Temporary: These functions should be removed and replaced with general support for Array<T> in Variant
inline Variant epToVariant(Slice<ResourceRef> resArray)
{
  Array<Variant> outs;
  for (auto &res : resArray)
    outs.pushBack(res);

  return outs;
}
inline void epFromVariant(const Variant &v, Array<const ResourceRef> *pResArray)
{
  Slice<Variant> ins = v.asArray();
  for (auto &res : ins)
    pResArray->pushBack(res.as<ResourceRef>());
}

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
  EP_DECLARE_COMPONENT_WITH_IMPL(ResourceManager, IResourceManager, Component, EPKERNEL_PLUGINVERSION, "ResourceManager desc...", 0)

public:
  // Resource getter/setters
  size_t GetNumResources() const override final { return pImpl->GetNumResources(); }
  void AddResource(ResourceRef res) override final { pImpl->AddResource(res); }
  void AddResourceArray(Slice<const ResourceRef> resArray) override final { pImpl->AddResourceArray(resArray); }
  void RemoveResource(ResourceRef res) override final { return pImpl->RemoveResource(res); }
  void RemoveResourceArray(Slice<const ResourceRef> resArray) override final { return pImpl->RemoveResourceArray(resArray); }

  void ClearResources() override final { pImpl->ClearResources(); }
  ResourceRef GetResource(String key) const override final { return pImpl->GetResource(key); }
  Array<ResourceRef> GetResourceArray() const override final { return pImpl->GetResourceArray(); }

  template<typename CT>
  SharedPtr<CT> GetResourceAs(String key) const
  {
    return component_cast<CT>(GetResource(key));
  }

  template<typename CT>
  Array<ResourceRef> GetResourcesByType() const
  {
    return GetResourcesByType(&CT::descriptor);
  }
  Array<ResourceRef> GetResourcesByType(const ep::ComponentDesc *pBase) const override final { return pImpl->GetResourcesByType(pBase); }

  ResourceIterator begin() const override final { return pImpl->begin(); }
  ResourceIterator end() const override final { return pImpl->end(); }

  Variant::VarMap GetExtensions() const override final { return pImpl->GetExtensions(); }
  // Resource loading/saving functions
  DataSourceRef LoadResourcesFromFile(Variant::VarMap initParams) override final { return pImpl->LoadResourcesFromFile(initParams); }
  void SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams) override final { pImpl->SaveResourcesToFile(spDataSource, initParams); }

  Variant Save() const override { return pImpl->Save(); }

  Event<Array<ResourceRef>> Added;
  Event<Array<ResourceRef>> Removed;

protected:
  void AddResourceArrayMethod(Array<const ResourceRef> resArray) { AddResourceArray(resArray); }
  void RemoveResourceArrayMethod(Array<const ResourceRef> resArray) { RemoveResourceArray(resArray); }

  ResourceManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(NumResources, "The number of Resources in the ResourceManager", nullptr, 0),
      EP_MAKE_PROPERTY_RO(ResourceArray, "An array populated with the ResourceManager's resources", nullptr, 0),
      EP_MAKE_PROPERTY_RO(Extensions, "The file extensions supported by the ResourceManager orgnanised by DataSource type", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(AddResource, "Add a Resource to the ResourceManager"),
      EP_MAKE_METHOD_EXPLICIT("AddResourceArray", AddResourceArrayMethod, "Add an Array of Resources to the ResourceManager"),
      EP_MAKE_METHOD(RemoveResource, "Remove the specified Resource"),
      EP_MAKE_METHOD_EXPLICIT("RemoveResourceArray", RemoveResourceArrayMethod, "Remove an Array Resources from the ResourceManager"),
      EP_MAKE_METHOD(ClearResources, "Remove all resources"),
      EP_MAKE_METHOD(GetResource, "Get Resource by UID"),
      EP_MAKE_METHOD(LoadResourcesFromFile, "Create a DataSource containing Resources from the file specified by the given File InitParams"),
      EP_MAKE_METHOD(SaveResourcesToFile, "Save Resources from the given DataSource to a file specified by the given File InitParams"),
    };
  }
  static Array<const EventInfo> GetEvents()
  {
    return{
      EP_MAKE_EVENT(Added, "Resources were added"),
      EP_MAKE_EVENT(Removed, "Resources were removed"),
    };
  }
};

} //namespace ep
#endif // EP_RESOURCE_MANAGER_H
