#include "components/datasources/uddatasource.h"
#include "components/resources/udmodel.h"
#include "components/resources/metadata.h"
#include "udOctree.h"
#include "kernel.h"

namespace ud
{
const udFixedSlice<const udString> UDDataSource::extensions = { ".uds", ".ssf", ".upc", ".udi", ".oct3" };

ComponentDesc UDDataSource::descriptor =
{
  &DataSource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION,      // udVersion
  UDSHELL_PLUGINVERSION,   // pluginVersion

  "uddataSource",     // id
  "UDDataSource",    // displayName
  "Provides UD Data", // description

  nullptr,            // properties
  nullptr,            // methods
  nullptr,            // events
  nullptr,            // static functions
  &RegisterExtensions // init
};

UDDataSource::UDDataSource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : DataSource(pType, pKernel, uid, initParams)
{
  const udVariant &source = initParams["src"];

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

      // Populate meta data
      int32_t count;
      result =  udOctree_GetMetadataCount(pOctree, &count);
      if (result == udR_Success)
      {
        for (int32_t i = 0; i < count; ++i)
        {
          const char *pName;
          const char *pValue;
          result = udOctree_GetMetadataByIndex(pOctree, i, &pName, &pValue, nullptr, nullptr);
          if (result == udR_Success)
          {
            MetadataRef meta = model->GetMetadata();
            meta->Insert(pName, pValue);
          }
        }
      }
    }
  }
}

udResult UDDataSource::RegisterExtensions(Kernel *pKernel)
{
  return pKernel->RegisterExtensions(&descriptor, extensions);
}

} // namespace ud
