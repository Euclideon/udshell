#pragma once
#ifndef EP_RESOURCE_MANAGER_H
#define EP_RESOURCE_MANAGER_H

#include "ep/cpp/component/component.h"

namespace ep
{
SHARED_CLASS(ResourceManager);
SHARED_CLASS(Resource);
SHARED_CLASS(DataSource);

class ResourceManager : public Component
{
  EP_DECLARE_COMPONENT(ResourceManager, Component, EPKERNEL_PLUGINVERSION, "ResourceManager desc...")
public:

  // Resource getter/setters
  size_t GetNumResources() const { return resources.Size(); }
  void AddResource(ResourceRef res);
  void RemoveResource(ResourceRef res);
  ResourceRef GetResource(String key) const { return *resources.Get(key); }
  template<typename CT>
  Array<ResourceRef> GetResourcesByType() const
  {
    return GetResourcesByType(&CT::descriptor);
  }
  Array<ResourceRef> GetResourcesByType(const ep::ComponentDesc *pBase) const;
  // TODO GetResourcesByPrefix with optional Type filter

  class Iterator;
  Iterator begin() const { return Iterator(resources.begin()); }
  Iterator end() const { return Iterator(resources.end()); }

  // Resource loading/saving functions
  DataSourceRef LoadResourcesFromFile(Variant::VarMap initParams);
  void SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams);

  class Iterator
  {
  public:
    Iterator(AVLTree<String, ResourceRef>::Iterator iter)
      : iter(iter)
    {}

    Iterator &operator++()
    {
      ++iter;
      return *this;
    }

    bool operator!=(Iterator rhs) { return iter != rhs.iter; }

    ResourceRef operator*() const { return (*iter).value; }

    AVLTree<String, ResourceRef>::Iterator iter;
  };
protected:
  ResourceManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~ResourceManager();

  AVLTree<String, ResourceRef> resources;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(NumResources, "The number of Resources in the ResourceManager", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods();
};

} //namespace ep
#endif // EP_RESOURCE_MANAGER_H
