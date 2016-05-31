#include "components/resources/udmodelimpl.h"
#include "components/datasources/udsource.h"
#include "enumkvp.h"

namespace ep {

static_assert(sizeof(internal::UDRenderPrivateData) == sizeof(udRenderModel), "sizeof(internal::UDRenderPrivateData) != sizeof(udRenderModel)");
static_assert(sizeof(internal::UDPrivateData) == sizeof(udOctree), "sizeof(internal::UDPrivateData) != sizeof(udOctree)");

Array<const PropertyInfo> UDModel::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(StartingRoot, "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", nullptr, 0),
    EP_MAKE_PROPERTY(AnimationFrame, "Animation frame number. Only valid for models that support animation", nullptr, 0),
    EP_MAKE_PROPERTY(RenderClipRect, "Clipping Rect of the Screen", nullptr, 0),
    EP_MAKE_PROPERTY(RenderFlags, "UD Rendering Flags", nullptr, 0),
    EP_MAKE_PROPERTY(UDMatrix, "UD Matrix", nullptr, 0),
    EP_MAKE_PROPERTY_RO(MemoryUsage, "Memory Usage", nullptr, 0),
    EP_MAKE_PROPERTY(VoxelFilterConstantData, "Voxel Filter Constant Data", nullptr, 0),
    EP_MAKE_PROPERTY(VoxelShaderConstantData, "Voxel Shader Constant Data", nullptr, 0),
    EP_MAKE_PROPERTY(PixelShaderConstantData, "Pixel Shader Constant Data", nullptr, 0),
    EP_MAKE_PROPERTY_EXPLICIT("VoxelShader", "Voxel Shader", EP_MAKE_GETTER(GetVarVoxelShader), EP_MAKE_SETTER(SetVarVoxelShader), nullptr, 0),
  };
}

UDModelImpl::~UDModelImpl()
{
  udOctree_Destroy(&pOctree);
}

void UDModelImpl::CopyRenderContext(UDRenderContext *pRenderContext) const
{
  EPASSERT(pRenderContext, "pRenderContext is NULL");

  udRenderModel *pRenderModel = union_reinterpret_cast<udRenderModel*>(pRenderContext);
  memset(pRenderModel, 0, sizeof(udRenderModel));

  pRenderModel->pOctree = pOctree;
  if (rectSet)
  {
    pRenderContext->privateClip.x0 = rect.x;
    pRenderContext->privateClip.y0 = rect.y;
    pRenderContext->privateClip.x1 = rect.x + rect.width;
    pRenderContext->privateClip.y1 = rect.y + rect.height;
    pRenderModel->pClip = union_reinterpret_cast<udRenderClipArea*>(&pRenderContext->privateClip);
  }

  pRenderContext->matrix = udmatrix;
  pRenderModel->pWorldMatrixD = pRenderContext->matrix.a;

  if (pVoxelFilter)
    pRenderModel->pFilter = (udRender_FilterFunc*)pVoxelFilter;

  if (pVoxelShader)
    pRenderModel->pVoxelShader = (udRender_VoxelShaderFunc*)pVoxelShader;
  else if (varVoxelShader)
    pRenderModel->pVoxelShader = VoxelVarShaderFunc;

  if (pPixelShader)
    pRenderModel->pPixelShader = (udRender_PixelShaderFunc*)pPixelShader;

  pRenderModel->flags = (udRenderFlags)renderFlags.v;
  pRenderModel->startingRoot = startingRoot;
  pRenderModel->animationFrame = animationFrame;

  pRenderContext->varVoxelShader = varVoxelShader;
}

uint32_t UDModelImpl::VoxelVarShaderFunc(udRenderModel *pRenderModel, udNodeIndex nodeIndex, udRenderNodeInfo *epUnusedParam(pNodeInfo))
{
  UDRenderContext *pContext = union_reinterpret_cast<UDRenderContext*>(pRenderModel);
  udOctree *pOctree = pRenderModel->pOctree;
  uint32_t color = pOctree->pGetNodeColor(pOctree, nodeIndex);
  color = (uint32_t)pContext->varVoxelShader(Slice<const Variant>{ Variant(color) }).asInt();
  return color;
}

} // namespace ep
