#include "components/resources/udmodelimpl.h"
#include "components/datasources/uddatasource.h"
#include "enumkvp.h"

namespace ep {

static const Slice<const EnumKVP> renderFlags =
{
  { "udRenderFlags::None", udRF_None },
  { "udRenderFlags::NoOrtho", udRF_NoOrtho },
  { "udRenderFlags::RenderMask", udRF_RenderMask },
  { "udRenderFlags::RenderImmediate", udRF_RenderImmediate },
  { "udRenderFlags::RenderCellBounds", udRF_RenderCellBounds },
  { "udRenderFlags::ClearTargets", udRF_ClearTargets },
  { "udRenderFlags::ForceZCompare", udRF_ForceZCompare },
  { "udRenderFlags::QuickTest", udRF_QuickTest },
  { "udRenderFlags::PointCubes", udRF_PointCubes },
  { "udRenderFlags::DisablePatch", udRF_DisablePatch },
  { "udRenderFlags::ForceSingleRoot", udRF_ForceSingleRoot },
  { "udRenderFlags::Patch2PixelOpt", udRF_Patch2PixelOpt },
  { "udRenderFlags::Transparent", udRF_Transparent }
};

UDModelImpl::~UDModelImpl()
{
  udOctree_Destroy(&pOctree);
}

UDRenderState UDModelImpl::GetUDRenderState() const
{
  UDRenderState state;
  memset(&state, 0, sizeof(state));

  state.matrix = udmatrix;

  state.voxelVarDelegate = voxelVarDelegate;

  state.flags = renderFlags;
  state.startingRoot = startingRoot;
  if (rectSet)
  {
    state.rect = rect;
    state.useClip = rectSet;
  }

  return state;
}

BoundingVolume UDModelImpl::GetBoundingVolume() const
{
  BoundingVolume vol;

  EPASSERT(udmatrix.a[0] == udmatrix.a[5] && udmatrix.a[0] == udmatrix.a[10], "NonUniform Scale");

  Double3 min = -(udmatrix.axis.t.toVector3());
  Double3 max = min + Double3::create(udmatrix.a[0]);

  // Theoretically there could be rotation so redoing min max
  vol.min.x = udMin(min.x, max.x);
  vol.min.y = udMin(min.y, max.y);
  vol.min.z = udMin(min.z, max.z);

  vol.max.x = udMax(min.x, max.x);
  vol.max.y = udMax(min.y, max.y);
  vol.max.z = udMax(min.z, max.z);

  return vol;
}

} // namespace ep
