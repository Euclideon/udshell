#include "components/resources/udmodelimpl.h"
#include "components/datasources/udsource.h"
#include "enumkvp.h"

namespace ep {

static_assert(sizeof(internal::UDRenderPrivateData) == sizeof(udRenderModel), "sizeof(internal::UDRenderPrivateData) != sizeof(udRenderModel)");
static_assert(sizeof(internal::UDPrivateData) == sizeof(udOctree), "sizeof(internal::UDPrivateData) != sizeof(udOctree)");

Array<const PropertyInfo> UDModel::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("startingRoot", GetStartingRoot, SetStartingRoot, "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", nullptr, 0),
    EP_MAKE_PROPERTY("animationFrame", GetAnimationFrame, SetAnimationFrame, "Animation frame number. Only valid for models that support animation", nullptr, 0),
    EP_MAKE_PROPERTY("renderClipRect", GetRenderClipRect, SetRenderClipRect, "Clipping Rect of the Screen", nullptr, 0),
    EP_MAKE_PROPERTY("renderFlags", GetRenderFlags, SetRenderFlags, "UD Rendering Flags", nullptr, 0),
    EP_MAKE_PROPERTY("udMatrix", GetUDMatrix, SetUDMatrix, "UD Matrix", nullptr, 0),
    EP_MAKE_PROPERTY("voxelFilterConstantData", GetVoxelFilterConstantData, SetVoxelFilterConstantData, "Voxel Filter Constant Data", nullptr, 0),
    EP_MAKE_PROPERTY("voxelShaderConstantData", GetVoxelShaderConstantData, SetVoxelShaderConstantData, "Voxel Shader Constant Data", nullptr, 0),
    EP_MAKE_PROPERTY("pixelShaderConstantData", GetPixelShaderConstantData, SetPixelShaderConstantData, "Pixel Shader Constant Data", nullptr, 0),
    EP_MAKE_PROPERTY("voxelShader", GetVarVoxelShader, SetVarVoxelShader, "Voxel Shader", nullptr, 0),
    EP_MAKE_PROPERTY_RO("memoryUsage", GetMemoryUsage, "Memory Usage", nullptr, 0),
  };
}

UDModelImpl::~UDModelImpl()
{
  udOctree_Destroy(&pOctree);
}

namespace internal {
union SharedModel
{
  UDRenderContext epRc;
  udRenderModel udRm;

  ~SharedModel() = delete; // Gets rid of warning
};
} // internal

void UDModelImpl::CopyRenderContext(UDRenderContext *pRenderContext) const
{
  EPASSERT(pRenderContext, "pRenderContext is NULL");

  internal::SharedModel *pShared = reinterpret_cast<internal::SharedModel*>(pRenderContext);
  memset(pShared, 0, sizeof(internal::SharedModel));

  pShared->udRm.pOctree = pOctree;
  if (rectSet)
  {
    pShared->epRc.privateClip.x0 = rect.x;
    pShared->epRc.privateClip.y0 = rect.y;
    pShared->epRc.privateClip.x1 = rect.x + rect.width;
    pShared->epRc.privateClip.y1 = rect.y + rect.height;
    pShared->udRm.pClip = reinterpret_cast<udRenderClipArea*>(&pShared->epRc.privateClip);
  }

  pShared->epRc.matrix = udmatrix;
  pShared->udRm.pWorldMatrixD = pShared->epRc.matrix.a;

  if (pVoxelFilter)
    pShared->udRm.pFilter = (udRender_FilterFunc*)pVoxelFilter;

  if (pVoxelShader)
    pShared->udRm.pVoxelShader = (udRender_VoxelShaderFunc*)pVoxelShader;
  else if (varVoxelShader)
    pShared->udRm.pVoxelShader = VoxelVarShaderFunc;

  if (pPixelShader)
    pShared->udRm.pPixelShader = (udRender_PixelShaderFunc*)pPixelShader;

  pShared->udRm.flags = (udRenderFlags)renderFlags.v;
  pShared->udRm.startingRoot = startingRoot;
  pShared->udRm.animationFrame = animationFrame;

  pShared->epRc.varVoxelShader = varVoxelShader;
}

uint32_t UDModelImpl::VoxelVarShaderFunc(udRenderModel *pRenderModel, udNodeIndex nodeIndex, udRenderNodeInfo *epUnusedParam(pNodeInfo))
{
  internal::SharedModel *pShared = reinterpret_cast<internal::SharedModel*>(pRenderModel);
  udOctree *pOctree = pShared->udRm.pOctree;
  uint32_t color = pOctree->pGetNodeColor(pOctree, nodeIndex);
  color = (uint32_t)pShared->epRc.varVoxelShader(Slice<const Variant>{ Variant(color) }).asInt();
  return color;
}

} // namespace ep
