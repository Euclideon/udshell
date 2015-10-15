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

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "resourcemanager", // id
  "Resource Manager", // displayName
  "Manages resource components", // description

  //Slice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  //Slice<CMethodDesc>(methods, UDARRAYSIZE(methods)), // methods
  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

ResourceManager::ResourceManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::AddResource(ResourceRef res) { resources.Insert(res->uid, res); }
void ResourceManager::RemoveResource(ResourceRef res) { resources.Remove(res->uid); }

Array<ResourceRef> ResourceManager::GetResourcesByType(const ComponentDesc *pBase) const
{
  Array<ResourceRef> outs;

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

void ResourceManager::LoadResourcesFromFile(InitParams initParams)
{
  Variant src = initParams["src"];
  String ext = src.asString().getRightAtLast('.');
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
  for (size_t i = 0; i < numResources; i++)
  {
    ResourceRef spResource = spDS->GetResource(i);
    resources.Insert(spResource->uid, spResource);
  }
}

void ResourceManager::SaveResourcesToFile(Slice<ResourceRef>, InitParams initParams)
{

}

} // namespace ep
