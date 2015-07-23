#include "udUDNode.h"
#include "udOctree.h"
#include "udRenderScene.h"

static const udSlice<const udEnumKVP> renderFlags =
{
  { "udRenderFlags::None",             udRF_None },
  { "udRenderFlags::NoOrtho",          udRF_NoOrtho },
  { "udRenderFlags::RenderMask",       udRF_RenderMask },
  { "udRenderFlags::RenderImmediate",  udRF_RenderImmediate },
  { "udRenderFlags::RenderCellBounds", udRF_RenderCellBounds },
  { "udRenderFlags::ClearTargets",     udRF_ClearTargets },
  { "udRenderFlags::ForceZCompare",    udRF_ForceZCompare },
  { "udRenderFlags::QuickTest",        udRF_QuickTest },
  { "udRenderFlags::PointCubes",       udRF_PointCubes },
  { "udRenderFlags::DisablePatch",     udRF_DisablePatch },
  { "udRenderFlags::ForceSingleRoot",  udRF_ForceSingleRoot },
  { "udRenderFlags::Patch2PixelOpt",   udRF_Patch2PixelOpt },
  { "udRenderFlags::Transparent",      udRF_Transparent }
};

static const udPropertyDesc props[] =
{
  {
    "startingroot", // id
    "Starting Root", // displayName
    "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", // description
    &udUDNode::GetStartingRoot,
    &udUDNode::SetStartingRoot,
    udTypeDesc(udPropertyType::Integer)
  },

  {
    "rendercliparea", // id
    "Render Clip Area", // displayName
    "Clipping Area of the Screen", // description
    &udUDNode::GetRenderClipArea,
    &udUDNode::SetRenderClipArea,
    udTypeDesc(udPropertyType::Struct)
  },

  {
    "renderflags", // id
    "Render Flags", // displayName
    "UD Rendering Flags", // description
    &udUDNode::GetRenderFlags,
    &udUDNode::SetRenderFlags,
    udTypeDesc(udPropertyType::Flags, 0, renderFlags)
  },
  {
    "datasource", // id
    "udModel Data Source", // displayName
    "Data Source for UD Model", // description
    &udUDNode::GetSource,
    nullptr,
    udTypeDesc(udPropertyType::String)
  },

  {
    "udscale", // id
    "UD Scale", // displayName
    "Internal Scale of the Model", // description
    &udUDNode::GetUDScale,
    nullptr,
    udTypeDesc(udPropertyType::Float)
  },

  {
    "getboundingvolume", // id
    "Get Bounding Volume", // displayName
    "Get the Bouning Volume", // description
    &udUDNode::GetBoundingVolume,
    nullptr,
    udTypeDesc(udPropertyType::Struct)
  },


#if 0
  {
    "VoxelShader", // id
    "Voxel Shader", // displayName
    "Optional callback to handle it's own internal call to GetNodeColor()", // description
    udPropertyType::Function, // type
    0, // arrayLength
    0, // flags
    udPropertyDisplayType::Default, // displayType
    nullptr, //udGetter(&udUDNode::GetVoxelShader),
    udSetter(&udUDNode::SetVoxelShader)
  },
  {
    "PixelShader", // id
    "Pixel Shader", // displayName
    "Optional callback to handle writing pixels and depth", // description
    udPropertyType::Function, // type
    0, // arrayLength
    0, // flags
    udPropertyDisplayType::Default, // displayType
    nullptr, //udGetter(&udUDNode::GetVoxelShader),
    udSetter(&udUDNode::SetVoxelShader)
}

#endif
};

static const udMethodDesc methods[] =
{
  {
    "load",
    "Load",
    "Load the UD Model",
    udMethod(&udUDNode::Load),
    udTypeDesc(udPropertyType::Integer), // result
    {
      udTypeDesc(udPropertyType::String),
      udTypeDesc(udPropertyType::Boolean)
    }
  }
};


const udComponentDesc udUDNode::descriptor =
{
  &udNode::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "pcNode",      // id
  "udUDNode",    // displayName
  "Is a udModel Node", // description

  [](){ return udR_Success; },             // pInit
  udUDNode::Create, // pCreateInstance

  udSlice<const udPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  udSlice<const udMethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};


int udUDNode::Load(udString name, bool useStreamer)
{
  source = "";
  udResult result;
  spModel = udSharedudModel::Create(name,useStreamer);
  if (!spModel)
  {
    result = udR_Failure_;
    UD_ERROR_HANDLE();
  }

  source = name;
  UD_ERROR_CHECK(udOctree_GetLocalMatrixF64(spModel->GetOctreePtr(), udMat.a));

epilogue:
  return (int)result;
}


udResult udUDNode::Render(udRenderScene *pScene, const udDouble4x4 &mat)
{
  udUDJob job;
  memset(&job, 0, sizeof(job));

  job.matrix = udMul(mat, udMat);
  job.renderModel.pOctree = spModel->GetOctreePtr();
  job.renderModel.pWorldMatrixD = job.matrix.a;
  job.renderModel.pVoxelShader = pVoxelShader;
  job.renderModel.pPixelShader = pPixelShader;
  job.renderModel.flags = renderFlags;
  job.renderModel.startingRoot = startingRoot;

  if (clipAreaSet)
  {
    job.clipArea = clipArea;
    job.renderModel.pClip = &job.clipArea;
  }

  pScene->ud.pushBack(job);
  return udR_Success;
}


udBoundingVolume udUDNode::GetBoundingVolume() const
{
  udBoundingVolume vol;

  UDASSERT(udMat.a[0] == udMat.a[5] == udMat.a[10], "NonUniform Scale");

  udDouble3 min = -(udMat.axis.t.toVector3());
  udDouble3 max = min + udDouble3::create(udMat.a[0]);

  // Theoretically there could be rotation so redoing min max
  vol.min.x = udMin(min.x, max.x);
  vol.min.y = udMin(min.y, max.y);
  vol.min.z = udMin(min.z, max.z);

  vol.max.x = udMax(min.x, max.x);
  vol.max.y = udMax(min.y, max.y);
  vol.max.z = udMax(min.z, max.z);

  return vol;
}
