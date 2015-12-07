#include "udmodel.h"
#include "components/datasources/uddatasource.h"

namespace kernel
{

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

UDModel::~UDModel()
{
  if (pOctree)
    pOctree->pDestroy(pOctree);
}

UDRenderState UDModel::GetUDRenderState() const
{
  UDRenderState renderState;

  memset(&renderState, 0, sizeof(renderState));

  renderState.matrix = udmatrix;

  renderState.pOctree = GetOctreePtr();
  renderState.pVoxelShader = pVoxelShader;
  renderState.pPixelShader = pPixelShader;
  renderState.flags = renderFlags;
  renderState.startingRoot = startingRoot;

  if (pVoxelShader)
    renderState.pVoxelShader = pVoxelShader;
  else if (simpleVoxelDel.GetMemento())
  {
    renderState.pVoxelShader = UDRenderState::VoxelShaderFunc;
    renderState.simpleVoxelDel = simpleVoxelDel;
  }

  if (rectSet)
  {
    udRenderClipArea area = { (uint32_t)rect.x, (uint32_t)rect.y,  (uint32_t)(rect.x + rect.width), (uint32_t)(rect.y + rect.height) };
    renderState.clipArea = area;
    renderState.pClip = &renderState.clipArea;
  }

  return renderState;
}

BoundingVolume UDModel::GetBoundingVolume() const
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

int UDModel::Load(String _name, bool useStreamer)
{
  epResult result = epR_Failure;
  if (!spDataSource)
  {
    spDataSource = GetKernel().CreateComponent<UDDataSource>({ { "src", _name },
                                                               { "useStreamer", useStreamer },
                                                               { "existingComponent",  (Component*)this }
                                                             } );
  }
  return (int)result;
}

} // namespace kernel
