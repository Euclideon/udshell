#include "components/datasources/uddatasource.h"
#include "components/resources/udmodel.h"
#include "components/resources/metadata.h"
#include "udOctree.h"
#include "kernel.h"

namespace ep
{
const Array<const String> UDDataSource::extensions = { ".uds", ".ssf", ".upc", ".udi", ".oct3" };

UDDataSource::UDDataSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : DataSource(pType, pKernel, uid, initParams)
{
  const Variant *source = initParams.Get("src");

  if (source && source->is(Variant::Type::String))
  {
    const Variant *useStreamer = initParams.Get("useStreamer");

    udOctree *pOctree = nullptr;
    udResult result = udOctree_Create(&pOctree, source->asString().toStringz(), useStreamer && useStreamer->is(Variant::Type::Bool) ? useStreamer->asBool() : true, 0);
    if (result == udR_Success)
    {
      epscope(fail) { udOctree_Destroy(&pOctree); };

      const Variant *udModel = initParams.Get("existingComponent");
      UDModelRef model;
      if (udModel && udModel->is(Variant::Type::Component))
        model = component_cast<UDModel>(udModel->asComponent());
      else
        model = pKernel->CreateComponent<UDModel>();

      // TODO: this need not exist if CreateComponent doesn't catch...
      if (!model)
        EPTHROW_ERROR(epR_Failure, "Failed to create model");

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

      result = udOctree_GetLocalMatrixF64(model->GetOctreePtr(), model->udmatrix.a);
      if (result == udR_Success)
        resources.Insert(source->asString(), model);
    }
  }
}

epResult UDDataSource::StaticInit(ep::Kernel *pKernel)
{
  return pKernel->RegisterExtensions(pKernel->GetComponentDesc(ComponentID()), extensions);
}

} // namespace ep
