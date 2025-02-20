#include "components/datasources/udsource.h"
#include "components/resources/udmodelimpl.h"
#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/component/file.h"
#include "udOctree.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/arraybuffer.h"

namespace ep
{
const Array<const String> UDSource::extensions = { ".uds", ".ssf", ".upc", ".udi", ".oct3" };

UDSource::UDSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : DataSource(pType, pKernel, uid, initParams)
{
  const Variant *source = initParams.get("src");

  if (source && source->is(Variant::Type::String))
  {
    const Variant *useStreamer = initParams.get("useStreamer");

    MutableString<260> filePath = File::urlToNativePath(source->asString());

    udOctree *pOctree = nullptr;
    udResult result = udOctree_Load(&pOctree, filePath.toStringz(), useStreamer && useStreamer->is(Variant::Type::Bool) ? useStreamer->asBool() : true, nullptr);
    EPTHROW_IF(result != udR_Success, Result::Failure, "Failed to Create UD model");

    epscope(fail) { udOctree_Destroy(&pOctree); };

    const Variant *udModel = initParams.get("existingComponent");
    UDModelRef model;
    if (udModel && udModel->is(Variant::SharedPtrType::Component))
      model = component_cast<UDModel>(udModel->asComponent());
    else
      model = pKernel->createComponent<UDModel>();

    UDModelImpl *pModelImpl = model->getImpl<UDModelImpl>();
    pModelImpl->pOctree = pOctree;

    // Populate meta data
    MetadataRef meta = model->getMetadata();
    int32_t count;
    result = udOctree_GetMetadataCount(pOctree, &count);
    if (result == udR_Success)
    {
      for (int32_t i = 0; i < count; ++i)
      {
        const char *pName;
        const char *pValue;
        result = udOctree_GetMetadataByIndex(pOctree, i, &pName, &pValue, nullptr, nullptr);
        if (result == udR_Success && !String("ModelAttributes").eq(pName))
          meta->insert(pName, pValue);
      }
    }

    udOctreeHeaderData headerData;
    result = udOctree_GetLocalMatrixF64(pModelImpl->pOctree, pModelImpl->udmatrix.a);
    EPASSERT_THROW(result == udR_Success, Result::Failure, "Failed to get udmodel's local matrix");

    result = udOctree_GetHeaderData(pOctree, &headerData);
    EPASSERT_THROW(result == udR_Success, Result::Failure, "Failed to get udmodel's header data");

    Variant::VarMap::MapType header;
    header.insert("scaledRange", headerData.scale);

    double *pBias = headerData.sourceBias;
    header.insert("baseOffset", Double3{ pBias[0], pBias[1], pBias[2] });

    double *pScale = headerData.sourceScale;
    header.insert("nonUniformScale", Double3{ pScale[0], pScale[1], pScale[2] });

    double *pOri = headerData.boundingBoxOrigin;
    double *pExt = headerData.boundingBoxExtents;

    Double3 centre = (pModelImpl->udmatrix * Double4{ pOri[0], pOri[1], pOri[2], 1.0 }).toVector3();
    Double3 extents = (pModelImpl->udmatrix * Double4{ pExt[0], pExt[1], pExt[2], 0.0 }).toVector3();

    header.insert("boundingVolume", BoundingVolume { centre - extents, centre + extents } );

    double *pPivot = headerData.pivotOrigin;
    pModelImpl->pivot = (pModelImpl->udmatrix * Double4{ pPivot[0], pPivot[1], pPivot[2], 1.0 }).toVector3();
    header.insert("pivot", pModelImpl->pivot);

    header.insert("unitMeterScale", headerData.unitMeterScale);
    header.insert("maxOctreeDepth", headerData.maxOctreeDepth);

    meta->insert("octreeHeader", std::move(header));

    setResource(source->asString(), model);

    Array<Variant> attribInfo;

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

      md.type = md.info.asString();

      md.offset = 0;
      Variant v(md);
      Variant::VarMap::MapType map = v.claimMap();
      map.insert("blend", UDAttributeBlend(descriptor.blendType));
      attribInfo.pushBack(std::move(map));
    }

    if (attribInfo.length)
      meta->insert("attributeInfo", std::move(attribInfo));
  }
}

void UDSource::staticInit(ep::Kernel *pKernel)
{
  pKernel->registerExtensions(pKernel->getComponentDesc(componentID()), extensions);
}

} // namespace ep
