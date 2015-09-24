#include "components/resourcemanager.h"
#include "components/resources/resource.h"
#include "components/datasource.h"
#include "kernel.h"

namespace ep
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

  //epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  //epSlice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

ResourceManager::ResourceManager(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::AddResource(ResourceRef res) { resources.Insert(res->uid, res); }
void ResourceManager::RemoveResource(ResourceRef res) { resources.Remove(res->uid); }

epArray<ResourceRef> ResourceManager::GetResourcesByType(const ComponentDesc *pBase) const
{
  epArray<ResourceRef> outs;

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

void ResourceManager::LoadResourcesFromFile(epInitParams initParams)
{
  epVariant src = initParams["src"];
  epString ext = src.asString().getRightAtLast('.');
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

void ResourceManager::SaveResourcesToFile(epSlice<ResourceRef>, epInitParams initParams)
{

}

} // namespace ep
