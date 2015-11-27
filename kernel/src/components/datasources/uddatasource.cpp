#include "components/datasources/uddatasource.h"
#include "components/resources/udmodel.h"
#include "components/resources/metadata.h"
#include "udOctree.h"
#include "kernel.h"

namespace kernel
{
const Array<const String> UDDataSource::extensions = { ".uds", ".ssf", ".upc", ".udi", ".oct3" };

UDDataSource::UDDataSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : DataSource(pType, pKernel, uid, initParams)
{
  const Variant &source = initParams["src"];

  if (source.is(Variant::Type::String))
  {
    const Variant &useStreamer = initParams["useStreamer"];

    udOctree *pOctree = nullptr;
    udResult result = udOctree_Create(&pOctree, source.asString().toStringz(), useStreamer.is(Variant::Type::Bool) ? useStreamer.asBool() : true, 0);
    if (result == udR_Success)
    {
      const Variant &udModel = initParams["existingComponent"];
      UDModelRef model = udModel.is(Variant::Type::Component) ? component_cast<UDModel>(udModel.asComponent()) : pKernel->CreateComponent<UDModel>();

      if (!model)
      {
        udOctree_Destroy(&pOctree);
        throw epR_Failure;
      }

      model->spDataSource = ComponentRef(this);
      model->pOctree = pOctree;

      // Populate meta data
      int32_t count;
      result = udOctree_GetMetadataCount(pOctree, &count);
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

      result = udOctree_GetLocalMatrixF64(model->GetOctreePtr(), model->udMat.a);
      if (result == udR_Success)
        resources.Insert(source.asString(), model);
    }
  }
}

epResult UDDataSource::StaticInit(ep::Kernel *pKernel)
{
  return pKernel->RegisterExtensions(pKernel->GetComponentDesc(ComponentID()), extensions);
}

} // namespace kernel
