#include "components/resourcemanagerimpl.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> ResourceManager::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("numResources", getNumResources, "The number of Resources in the ResourceManager", nullptr, 0),
    EP_MAKE_PROPERTY_RO("resourceArray", getResourceArray, "An array populated with the ResourceManager's resources", nullptr, 0),
    EP_MAKE_PROPERTY_RO("extensions", getExtensions, "The file extensions supported by the ResourceManager orgnanised by DataSource type", nullptr, 0),
  };
}
Array<const MethodInfo> ResourceManager::getMethods() const
{
  return{
    EP_MAKE_METHOD(addResource, "Add a Resource to the ResourceManager"),
    EP_MAKE_METHOD_EXPLICIT("addResourceArray", addResourceArrayMethod, "Add an Array of Resources to the ResourceManager"),
    EP_MAKE_METHOD(removeResource, "Remove the specified Resource"),
    EP_MAKE_METHOD_EXPLICIT("removeResourceArray", removeResourceArrayMethod, "Remove an Array Resources from the ResourceManager"),
    EP_MAKE_METHOD(clearResources, "Remove all resources"),
    EP_MAKE_METHOD(getResource, "Get Resource by UID"),
    EP_MAKE_METHOD(loadResourcesFromFile, "Create a DataSource containing Resources from the file specified by the given File InitParams"),
    EP_MAKE_METHOD(saveResourcesToFile, "Save Resources from the given DataSource to a file specified by the given File InitParams"),
  };
}
Array<const EventInfo> ResourceManager::getEvents() const
{
  return{
    EP_MAKE_EVENT(added, "Resources were added"),
    EP_MAKE_EVENT(removed, "Resources were removed"),
  };
}


void ResourceManagerImpl::AddResourceArray(Slice<const ResourceRef> resArray)
{
  for(auto &res : resArray)
    resources.insert(res->uid, res);

  pInstance->added.Signal(resArray);
}

void ResourceManagerImpl::RemoveResourceArray(Slice<const ResourceRef> resArray)
{
  for (auto &res : resArray)
    resources.remove(res->uid);

  pInstance->removed.Signal(resArray);
}

void ResourceManagerImpl::ClearResources()
{
  Array<const ResourceRef> resArray(Reserve, resources.size());

  for (auto kvp : resources) {
    resArray.pushBack(kvp.value);
  }

  pInstance->removed.Signal(resArray);

  resources = nullptr;
}

Variant::VarMap ResourceManagerImpl::GetExtensions() const
{
  const AVLTree<String, const ep::ComponentDesc *> &extensionsRegistry = GetKernel()->getExtensionsRegistry();
  AVLTree<SharedString, Array<SharedString>> exts;
  Variant::VarMap map;

  for (auto kvp : extensionsRegistry)
  {
    Array<SharedString> *pCompExts = exts.get(kvp.value->info.identifier);
    if (pCompExts)
      pCompExts->pushBack(kvp.key);
    else
      exts.insert(kvp.value->info.identifier, { kvp.key });
  }

  for (auto item : exts)
    map.insert(item.key, item.value);

  return map;
}

Array<ResourceRef> ResourceManagerImpl::GetResourcesByType(const ep::ComponentDesc *pBase) const
{
  Array<ResourceRef> outs;

  for (auto kvp : resources)
  {
    const ep::ComponentDesc *pDesc = kvp.value->getDescriptor();

    while (pDesc)
    {
      if (pDesc == pBase)
      {
        outs.concat(kvp.value);
        break;
      }
      pDesc = pDesc->pSuperDesc;
    }
  }

  return outs;
}

Array<ResourceRef> ResourceManagerImpl::GetResourceArray() const
{
  Array<ResourceRef> outs(Reserve, resources.size());

  for (auto kvp : resources)
    outs.pushBack(kvp.value);

  return outs;
}

DataSourceRef ResourceManagerImpl::LoadResourcesFromFile(Variant::VarMap initParams)
{
  Variant src = *initParams.get("src");
  String ext = src.asString().getRightAtLast('.');
  if (ext.empty())
    EPTHROW_WARN(Result::InvalidArgument, 2, "LoadResourcesFromFile - \"src\" parameter is invalid");

  DataSourceRef spDS;
  epscope(fail) { if (!spDS) logWarning(2, "LoadResourcesFromFile - \"src\" file not found or not supported: {0}", src.asString()); };
  spDS = GetKernel()->createDataSourceFromExtension(ext, initParams);

  size_t numResources = spDS->getNumResources();
  Array<ResourceRef> resArray(Reserve, numResources);
  for (size_t i = 0; i < numResources; i++)
    resArray.pushBack(spDS->getResource(i));

  AddResourceArray(resArray);

  return spDS;
}

void ResourceManagerImpl::SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams)
{
  // TODO: Implement this function when needed
}

} // namespace ep
