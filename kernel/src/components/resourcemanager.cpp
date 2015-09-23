#include "components/resourcemanager.h"
#include "components/resources/resource.h"
#include "components/datasource.h"
#include "kernel.h"

namespace ud
{

/*
static CMethodDesc methods[] =
{

};
static CPropertyDesc props[] =
{

};
*/

ComponentDesc ResourceManager::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "resourcemanager", // id
  "Resource Manager", // displayName
  "Manages resource components", // description

  //udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  //udSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

ResourceManager::ResourceManager(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::AddResource(ResourceRef res) { resources.Insert(res->uid, res); }
void ResourceManager::RemoveResource(ResourceRef res) { resources.Remove(res->uid); }

udFixedSlice<ResourceRef> ResourceManager::GetResourcesByType(const ComponentDesc *pBase) const
{
  udFixedSlice<ResourceRef> outs;

  for (ResourceRef spRes : resources)
  {
    const ComponentDesc *pDesc = spRes->pType;

    while (pDesc)
    {
      if (pDesc == pBase)
      {
        outs.concat(spRes);
        break;
      }
      pDesc = pDesc->pSuperDesc;
    }
  }

  return outs;
}

void ResourceManager::LoadResourcesFromFile(udInitParams initParams)
{
  udVariant src = initParams["src"];
  udString ext = src.asString().getRightAtLast('.');
  if (ext.empty())
  {
    LogWarning(2, "LoadResourcesFromFile - \"src\" parameter is invalid");
    return; // TODO Handle error
  }

  DataSourceRef spDS = pKernel->CreateDataSourceFromExtension(ext, initParams);
  if (!spDS)
  {
    LogWarning(2, "LoadResourcesFromFile - \"src\" file not found: {0}", src.asString());
    return; // TODO Fix error returns
  }

  size_t numResources = spDS->GetNumResources();
  for (int i = 0; i < numResources; i++)
  {
    ResourceRef spResource = spDS->GetResource(i);
    resources.Insert(spResource->uid, spResource);
  }
}

void ResourceManager::SaveResourcesToFile(udSlice<ResourceRef>, udInitParams initParams)
{

}

} // namespace ud
