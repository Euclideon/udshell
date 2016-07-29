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
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, ResourceManager, IResourceManager, Component, EPKERNEL_PLUGINVERSION, "ResourceManager desc...", 0)

public:
  // Resource getter/setters
  size_t getNumResources() const { return pImpl->GetNumResources(); }
  void addResource(ResourceRef res) { pImpl->AddResource(res); }
  void addResourceArray(Slice<const ResourceRef> resArray) { pImpl->AddResourceArray(resArray); }
  void removeResource(ResourceRef res) { return pImpl->RemoveResource(res); }
  void removeResourceArray(Slice<const ResourceRef> resArray) { return pImpl->RemoveResourceArray(resArray); }

  void clearResources() { pImpl->ClearResources(); }
  ResourceRef getResource(String key) const { return pImpl->GetResource(key); }
  Array<ResourceRef> getResourceArray() const { return pImpl->GetResourceArray(); }

  template<typename CT>
  SharedPtr<CT> getResourceAs(String key) const
  {
    return component_cast<CT>(getResource(key));
  }

  template<typename CT>
  Array<ResourceRef> getResourcesByType() const
  {
    return getResourcesByType(&CT::descriptor);
  }
  Array<ResourceRef> getResourcesByType(const ep::ComponentDesc *pBase) const { return pImpl->GetResourcesByType(pBase); }

  ResourceIterator begin() const { return pImpl->begin(); }
  ResourceIterator end() const { return pImpl->end(); }

  Variant::VarMap getExtensions() const { return pImpl->GetExtensions(); }
  // Resource loading/saving functions
  DataSourceRef loadResourcesFromFile(Variant::VarMap initParams) { return pImpl->LoadResourcesFromFile(initParams); }
  void saveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams) { pImpl->SaveResourcesToFile(spDataSource, initParams); }

  Variant save() const override { return pImpl->Save(); }

  Event<Array<ResourceRef>> added;
  Event<Array<ResourceRef>> removed;

protected:
  void addResourceArrayMethod(Array<const ResourceRef> resArray) { addResourceArray(resArray); }
  void removeResourceArrayMethod(Array<const ResourceRef> resArray) { removeResourceArray(resArray); }

  ResourceManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
  Array<const EventInfo> getEvents() const;
};

} //namespace ep

#endif // EP_RESOURCE_MANAGER_H
