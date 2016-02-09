#include "components/resourcemanagerimpl.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "kernel.h"

namespace ep {

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
  epscope(fail) { if (!spDS) LogWarning(2, "LoadResourcesFromFile - \"src\" file not found: {0}", src.asString()); };
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
