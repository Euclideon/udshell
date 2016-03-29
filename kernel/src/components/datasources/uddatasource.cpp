#include "components/datasources/uddatasource.h"
#include "components/resources/udmodelimpl.h"
#include "ep/cpp/component/resource/metadata.h"
#include "components/file.h"
#include "udOctree.h"
#include "ep/cpp/kernel.h"

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

    MutableString<260> filePath = File::UrlToNativePath(source->asString());

    udOctree *pOctree = nullptr;
    udResult result = udOctree_Load(&pOctree, filePath.toStringz(), useStreamer && useStreamer->is(Variant::Type::Bool) ? useStreamer->asBool() : true, 0);
    EPTHROW_IF(result != udR_Success, epR_Failure, "Failed to Create UD model");

    epscope(fail) { udOctree_Destroy(&pOctree); };

    const Variant *udModel = initParams.Get("existingComponent");
    UDModelRef model;
    if (udModel && udModel->is(Variant::SharedPtrType::Component))
      model = component_cast<UDModel>(udModel->asComponent());
    else
      model = pKernel->CreateComponent<UDModel>();

    UDModelImpl *pModelImpl = model->GetImpl<UDModelImpl>();
    pModelImpl->pOctree = pOctree;

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

    result = udOctree_GetLocalMatrixF64(pModelImpl->pOctree, pModelImpl->udmatrix.a);
    if (result == udR_Success)
      SetResource(source->asString(), model);
  }
}

void UDDataSource::StaticInit(ep::Kernel *pKernel)
{
  pKernel->RegisterExtensions(pKernel->GetComponentDesc(ComponentID()), extensions);
}

} // namespace ep
