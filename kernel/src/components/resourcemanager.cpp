#include "components/resourcemanager.h"
#include "components/resources/resource.h"
#include "components/datasource.h"
#include "kernel.h"

namespace kernel
{

ResourceManager::ResourceManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::AddResource(ResourceRef res) { resources.Insert(res->uid, res); }
void ResourceManager::RemoveResource(ResourceRef res) { resources.Remove(res->uid); }

Array<ResourceRef> ResourceManager::GetResourcesByType(const ep::ComponentDesc *pBase) const
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

DataSourceRef ResourceManager::LoadResourcesFromFile(InitParams initParams)
{
  Variant src = initParams["src"];
  String ext = src.asString().getRightAtLast('.');
  if (ext.empty())
  {
    LogWarning(2, "LoadResourcesFromFile - \"src\" parameter is invalid");
    return nullptr; // TODO Handle error
  }

  DataSourceRef spDS = GetKernel().CreateDataSourceFromExtension(ext, initParams);
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

void ResourceManager::SaveResourcesToFile(Slice<ResourceRef>, InitParams initParams)
{

}

} // namespace kernel
