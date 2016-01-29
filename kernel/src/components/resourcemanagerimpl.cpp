#include "components/resourcemanagerimpl.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "kernel.h"

namespace ep {

void ResourceManagerImpl::AddResource(ResourceRef res) { resources.Insert(res->uid, res); }
void ResourceManagerImpl::RemoveResource(ResourceRef res) { resources.Remove(res->uid); }

Array<ResourceRef> ResourceManagerImpl::GetResourcesByType(const ep::ComponentDesc *pBase) const
{
  Array<ResourceRef> outs;

  for (auto spRes : resources)
  {
    const ep::ComponentDesc *pDesc = spRes.value->GetDescriptor();

    while (pDesc)
    {
      if (pDesc == pBase)
      {
        outs.concat(spRes.value);
        break;
      }
      pDesc = pDesc->pSuperDesc;
    }
  }

  return outs;
}

DataSourceRef ResourceManagerImpl::LoadResourcesFromFile(Variant::VarMap initParams)
{
  Variant src = *initParams.Get("src");
  String ext = src.asString().getRightAtLast('.');
  if (ext.empty())
  {
    LogWarning(2, "LoadResourcesFromFile - \"src\" parameter is invalid");
    return nullptr; // TODO Handle error
  }

  DataSourceRef spDS = GetKernel()->CreateDataSourceFromExtension(ext, initParams);
  if (!spDS)
  {
    LogWarning(2, "LoadResourcesFromFile - \"src\" file not found: {0}", src.asString());
    return nullptr; // TODO Fix error returns
  }

  size_t numResources = spDS->GetNumResources();
  for (size_t i = 0; i < numResources; i++)
  {
    ResourceRef spResource = spDS->GetResource(i);
    resources.Insert(spResource->uid, spResource);
  }

  return spDS;
}

void ResourceManagerImpl::SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams)
{
  // TODO: Implement this function when needed
}

} // namespace ep
