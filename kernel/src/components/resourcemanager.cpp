#include "components/resourcemanager.h"
#include "ep/cpp/component/resource/resource.h"
#include "components/datasource.h"
#include "kernel.h"

namespace ep
{

ResourceManager::ResourceManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

ResourceManager::~ResourceManager()
{

}

Array<const MethodInfo> ResourceManager::GetMethods()
{
  return{
    EP_MAKE_METHOD(AddResource, "Add a Resource to the ResourceManager"),
    EP_MAKE_METHOD(RemoveResource, "Remove the specified Resource"),
    EP_MAKE_METHOD(GetResource, "Get Resource by UID"),
    EP_MAKE_METHOD(LoadResourcesFromFile, "Create a DataSource containing Resources from the file specified by the given File InitParams"),
    EP_MAKE_METHOD(SaveResourcesToFile, "Save Resources from the given DataSource to a file specified by the given File InitParams"),
  };
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

DataSourceRef ResourceManager::LoadResourcesFromFile(Variant::VarMap initParams)
{
  Variant src = *initParams.Get("src");
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

void ResourceManager::SaveResourcesToFile(DataSourceRef spDataSource, Variant::VarMap initParams)
{
  // TODO: Implement this function when needed
}

} // namespace ep
