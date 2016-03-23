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

class ResourceManager : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ResourceManager, IResourceManager, Component, EPKERNEL_PLUGINVERSION, "ResourceManager desc...", 0)

public:
  // Resource getter/setters
  size_t GetNumResources() const { return pImpl->GetNumResources(); }
  void AddResource(ResourceRef res) { pImpl->AddResource(res); }
  void AddResourceArray(Slice<const ResourceRef> resArray) { pImpl->AddResourceArray(resArray); }
  void RemoveResource(ResourceRef res) { return pImpl->RemoveResource(res); }
  void RemoveResourceArray(Slice<const ResourceRef> resArray) { return pImpl->RemoveResourceArray(resArray); }

  void ClearResources() { pImpl->ClearResources(); }
  ResourceRef GetResource(String key) const { return pImpl->GetResource(key); }
  Array<ResourceRef> GetResourceArray() const { return pImpl->GetResourceArray(); }

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
  Array<ResourceRef> GetResourcesByType(const ep::ComponentDesc *pBase) const { return pImpl->GetResourcesByType(pBase); }

  ResourceIterator begin() const { return pImpl->begin(); }
  ResourceIterator end() const { return pImpl->end(); }

  Variant::VarMap GetExtensions() const { return pImpl->GetExtensions(); }
  // Resource loading/saving functions
  DataSourceRef LoadResourcesFromFile(Variant::VarMap initParams) { return pImpl->LoadResourcesFromFile(initParams); }
  void SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams) { pImpl->SaveResourcesToFile(spDataSource, initParams); }

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

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
  Array<const EventInfo> GetEvents() const;
};

} //namespace ep

#endif // EP_RESOURCE_MANAGER_H
