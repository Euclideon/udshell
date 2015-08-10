#include "components/datasources/UDDataSource.h"
#include "components/resources/udModel.h"
#include "udOctree.h"

namespace ud
{

ComponentDesc UDDataSource::descriptor =
{
  &DataSource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "UDDataSource", // id
  "UDData Source", // displayName
  "Provides UD Data", // description
};

UDDataSource::UDDataSource(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
  : DataSource(pType, pKernel, uid, initParams)
{
  const udVariant &source = initParams["source"];

  if (source.is(udVariant::Type::String))
  {
    const udVariant &useStreamer = initParams["useStreamer"];

    udOctree *pOctree = nullptr;
    udResult result = udOctree_Create(&pOctree, source.asString().toStringz(), useStreamer.is(udVariant::Type::Bool) ? useStreamer.asBool() : true, 0);
    if (result == udR_Success)
    {
      UDModelRef model = pKernel->CreateComponent<UDModel>();
      model->pOctree = pOctree;
      resources.Insert(source.asString(), model);
    }
  }
}

} // namespace ud
