#include "components/resourcemanagerimpl.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> ResourceManager::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(NumResources, "The number of Resources in the ResourceManager", nullptr, 0),
    EP_MAKE_PROPERTY_RO(ResourceArray, "An array populated with the ResourceManager's resources", nullptr, 0),
    EP_MAKE_PROPERTY_RO(Extensions, "The file extensions supported by the ResourceManager orgnanised by DataSource type", nullptr, 0),
  };
}
Array<const MethodInfo> ResourceManager::GetMethods() const
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
Array<const EventInfo> ResourceManager::GetEvents() const
{
  return{
    EP_MAKE_EVENT(Added, "Resources were added"),
    EP_MAKE_EVENT(Removed, "Resources were removed"),
  };
}


void ResourceManagerImpl::AddResourceArray(Slice<const ResourceRef> resArray)
{
  for(auto &res : resArray)
    resources.Insert(res->uid, res);

  pInstance->Added.Signal(resArray);
}

void ResourceManagerImpl::RemoveResourceArray(Slice<const ResourceRef> resArray)
{
  for (auto &res : resArray)
    resources.Remove(res->uid);

  pInstance->Removed.Signal(resArray);
}

void ResourceManagerImpl::ClearResources()
{
  Array<const ResourceRef> resArray(Reserve, resources.Size());

  for (auto kvp : resources) {
    resArray.pushBack(kvp.value);
  }

  pInstance->Removed.Signal(resArray);

  resources = nullptr;
}

Variant::VarMap ResourceManagerImpl::GetExtensions() const
{
  const AVLTree<String, const ep::ComponentDesc *> &extensionsRegistry = GetKernel()->GetExtensionsRegistry();
  AVLTree<SharedString, Array<SharedString>> exts;
  Variant::VarMap map;

  for (auto kvp : extensionsRegistry)
  {
    Array<SharedString> *pCompExts = exts.Get(kvp.value->info.id);
    if (pCompExts)
      pCompExts->pushBack(kvp.key);
    else
      exts.Insert(kvp.value->info.id, { kvp.key });
  }

  for (auto item : exts)
    map.Insert(item.key, item.value);

  return map;
}

Array<ResourceRef> ResourceManagerImpl::GetResourcesByType(const ep::ComponentDesc *pBase) const
{
  Array<ResourceRef> outs;

  for (auto kvp : resources)
  {
    const ep::ComponentDesc *pDesc = kvp.value->GetDescriptor();

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
  Array<ResourceRef> outs(Reserve, resources.Size());

  for (auto kvp : resources)
    outs.pushBack(kvp.value);

  return outs;
}

DataSourceRef ResourceManagerImpl::LoadResourcesFromFile(Variant::VarMap initParams)
{
  Variant src = *initParams.Get("src");
  String ext = src.asString().getRightAtLast('.');
  if (ext.empty())
    EPTHROW_WARN(epR_InvalidArgument, 2, "LoadResourcesFromFile - \"src\" parameter is invalid");

  DataSourceRef spDS;
  epscope(fail) { if (!spDS) LogWarning(2, "LoadResourcesFromFile - \"src\" file not found or not supported: {0}", src.asString()); };
  spDS = GetKernel()->CreateDataSourceFromExtension(ext, initParams);

  size_t numResources = spDS->GetNumResources();
  Array<ResourceRef> resArray(Reserve, numResources);
  for (size_t i = 0; i < numResources; i++)
    resArray.pushBack(spDS->GetResource(i));

  AddResourceArray(resArray);

  return spDS;
}

void ResourceManagerImpl::SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams)
{
  // TODO: Implement this function when needed
}

} // namespace ep
