#pragma once
#if !defined(_EP_IRESOURCEMANAGER_HPP)
#define _EP_IRESOURCEMANAGER_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(Resource);
SHARED_CLASS(DataSource);

class ResourceIterator;

struct ComponentDesc;

class IResourceManager
{
public:
  // Resource getter/setters
  virtual size_t GetNumResources() const = 0;
  virtual void AddResource(ResourceRef res) = 0;
  virtual void RemoveResource(ResourceRef res) = 0;
  virtual ResourceRef GetResource(String key) const = 0;
  virtual Array<ResourceRef> GetResourcesByType(const ep::ComponentDesc *pBase) const = 0;

  virtual ResourceIterator begin() const = 0;
  virtual ResourceIterator end() const = 0;

  // Resource loading/saving functions
  virtual DataSourceRef LoadResourcesFromFile(Variant::VarMap initParams) = 0;
  virtual void SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams) = 0;

  virtual Variant Save() const = 0;
};

} //namespace ep
#endif // EP_IRESOURCEMANAGER_H
