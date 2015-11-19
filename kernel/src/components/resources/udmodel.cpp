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

static CPropertyDesc props[] =
{
  {
    {
      "startingroot", // id
      "Starting Root", // displayName
      "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", // description
    },
    &UDModel::GetStartingRoot,
    &UDModel::SetStartingRoot
  },
  {
    {
      "rendercliprect", // id
      "Render Clip Rect", // displayName
      "Clipping Rect of the Screen", // description
    },
    &UDModel::GetRenderClipRect,
    &UDModel::SetRenderClipRect
  },
  {
    {
      "renderflags", // id
      "Render Flags", // displayName
      "UD Rendering Flags", // description
    },
    &UDModel::GetRenderFlags,
    &UDModel::SetRenderFlags
  },
  {
    {
      "datasource", // id
      "udModel Data Source", // displayName
      "Data Source for UD Model", // description
    },
    &UDModel::GetDataSource,
    nullptr,
  },
  {
    {
      "udscale", // id
      "UD Scale", // displayName
      "Internal Scale of the Model", // description
    },
    &UDModel::GetUDScale,
    nullptr
  },

  {
    {
      "boundingvolume", // id
      "Bounding Volume", // displayName
      "The Bouning Volume", // description
    },
    &UDModel::GetBoundingVolume,
    nullptr
  },
  {
    {
      "voxelshader", // id
      "Voxel Shader", // displayName
      "Optional callback to handle it's own internal call to GetNodeColor()", // description
    },
    &UDModel::GetSimpleVoxelDelegate,
    &UDModel::SetSimpleVoxelDelegate
  },
#if 0
  {
    {
      "PixelShader", // id
      "Pixel Shader", // displayName
      "Optional callback to handle writing pixels and depth", // description
    },
    nullptr, //Getter(&UDNode::GetVoxelShader),
    Setter(&UDModel::SetVoxelShader)
  }
#endif
};

ComponentDesc UDModel::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "udmodel", // id
  "UD Model", // displayName
  "UD model resource", // description

  Slice<CPropertyDesc>(props, EPARRAYSIZE(props)), // properties
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

  renderState.matrix = udMat;

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

  EPASSERT(udMat.a[0] == udMat.a[5] && udMat.a[0] == udMat.a[10], "NonUniform Scale");

  Double3 min = -(udMat.axis.t.toVector3());
  Double3 max = min + Double3::create(udMat.a[0]);

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
    spDataSource = pKernel->CreateComponent<UDDataSource>({ { "src", _name },
                                                            { "useStreamer", useStreamer },
                                                            { "existingComponent",  (Component*)this }
                                                          } );
  }
  return (int)result;
}

} // namespace kernel
