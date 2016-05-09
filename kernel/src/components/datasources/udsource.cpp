#include "components/datasources/udsource.h"
#include "components/resources/udmodelimpl.h"
#include "ep/cpp/component/resource/metadata.h"
#include "components/file.h"
#include "udOctree.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/arraybuffer.h"

namespace ep
{
const Array<const String> UDSource::extensions = { ".uds", ".ssf", ".upc", ".udi", ".oct3" };

UDSource::UDSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : DataSource(pType, pKernel, uid, initParams)
{
  const Variant *source = initParams.Get("src");

  if (source && source->is(Variant::Type::String))
  {
    const Variant *useStreamer = initParams.Get("useStreamer");

    MutableString<260> filePath = File::UrlToNativePath(source->asString());

    udOctree *pOctree = nullptr;
    udResult result = udOctree_Load(&pOctree, filePath.toStringz(), useStreamer && useStreamer->is(Variant::Type::Bool) ? useStreamer->asBool() : true, nullptr);
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
    MetadataRef meta = model->GetMetadata();
    if (result == udR_Success)
    {
      for (int32_t i = 0; i < count; ++i)
      {
        const char *pName;
        const char *pValue;
        result = udOctree_GetMetadataByIndex(pOctree, i, &pName, &pValue, nullptr, nullptr);
        if (result == udR_Success)
        {
          meta->Insert(pName, pValue);
        }
      }
    }

    result = udOctree_GetLocalMatrixF64(pModelImpl->pOctree, pModelImpl->udmatrix.a);
    if (result == udR_Success)
      SetResource(source->asString(), model);

    Array<Variant> varMetadata;

    for (udStreamType i = udST_RawAttributeFirst; i <= udST_RawAttributeLast; i = udStreamType(i + 1))
    {
      udAttributeDescriptor descriptor;
      result = udOctree_GetAttributeDescriptor(pOctree, i, &descriptor);
      if (result != udR_Success)
        break;

      ElementMetadata md;
      md.name = descriptor.name;

      md.info.size = (descriptor.typeInfo & udATI_SizeMask) >> udATI_SizeShift;
      size_t numComponents = (descriptor.typeInfo & udATI_ComponentCountMask) >> udATI_ComponentCountShift;
      if (numComponents > 1)
        md.info.dimensions = SharedArray<size_t>{ numComponents };

      md.info.flags |= descriptor.typeInfo & udATI_Float ? ElementInfoFlags::Float : 0;
      md.info.flags |= descriptor.typeInfo & udATI_Signed ? ElementInfoFlags::Signed : 0;
      md.info.flags |= descriptor.typeInfo & udATI_Color ? ElementInfoFlags::Color : 0;

      md.type = md.info.AsString();

      md.offset = 0;
      varMetadata.pushBack(md);
      varMetadata[size_t(i - udST_RawAttributeFirst)].insertItem("blend", UDAttributeBlend(descriptor.blendType));
    }

    if (varMetadata.length)
      meta->Insert("attributeinfo", std::move(varMetadata));
  }
}

void UDSource::StaticInit(ep::Kernel *pKernel)
{
  pKernel->RegisterExtensions(pKernel->GetComponentDesc(ComponentID()), extensions);
}

} // namespace ep
