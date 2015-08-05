#include "udnode.h"
#include "udOctree.h"
#include "renderscene.h"
#include "kernel.h"
#include "udRender.h"
#include "util/uddelegate.h"

namespace ud
{

static const udSlice<const EnumKVP> renderFlags =
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

static PropertyDesc props[] =
{
  {
    "startingroot", // id
    "Starting Root", // displayName
    "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", // description
    &UDNode::GetStartingRoot,
    &UDNode::SetStartingRoot,
    TypeDesc(PropertyType::Integer)
  },

  {
    "rendercliparea", // id
    "Render Clip Area", // displayName
    "Clipping Area of the Screen", // description
    &UDNode::GetRenderClipArea,
    &UDNode::SetRenderClipArea,
    TypeDesc(PropertyType::Struct)
  },

  {
    "renderflags", // id
    "Render Flags", // displayName
    "UD Rendering Flags", // description
    &UDNode::GetRenderFlags,
    &UDNode::SetRenderFlags,
    TypeDesc(PropertyType::Flags, 0, renderFlags)
  },
  {
    "datasource", // id
    "udModel Data Source", // displayName
    "Data Source for UD Model", // description
    &UDNode::GetSource,
    nullptr,
    TypeDesc(PropertyType::String)
  },

  {
    "udscale", // id
    "UD Scale", // displayName
    "Internal Scale of the Model", // description
    &UDNode::GetUDScale,
    nullptr,
    TypeDesc(PropertyType::Float)
  },

  {
    "boundingvolume", // id
    "Bounding Volume", // displayName
    "The Bouning Volume", // description
    &UDNode::GetBoundingVolume,
    nullptr,
    TypeDesc(PropertyType::Struct)
  },


  {
    "voxelshader", // id
    "Voxel Shader", // displayName
    "Optional callback to handle it's own internal call to GetNodeColor()", // description
    &UDNode::GetSimpleVoxelDelegate,
    &UDNode::SetSimpleVoxelDelegate,
    PropertyType::Delegate, // type
  },
#if 0
  {
    "PixelShader", // id
    "Pixel Shader", // displayName
    "Optional callback to handle writing pixels and depth", // description
    PropertyType::Function, // type
    0, // arrayLength
    0, // flags
    udPropertyDisplayType::Default, // displayType
    nullptr, //Getter(&UDNode::GetVoxelShader),
    Setter(&UDNode::SetVoxelShader)
}

#endif
};

static MethodDesc methods[] =
{
  {
    "load",
    "Load the UD Model",
    Method(&UDNode::Load),
    TypeDesc(PropertyType::Integer), // result
    {
      TypeDesc(PropertyType::String),
      TypeDesc(PropertyType::Boolean)
    }
  }
};


ComponentDesc UDNode::descriptor =
{
  &Node::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "udnode",    // id
  "Node",  // displayName
  "Is a udModel Node", // description

  udSlice<PropertyDesc>(props, UDARRAYSIZE(props)), // properties
  udSlice<MethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};


int UDNode::Load(udString name, bool useStreamer)
{
  udResult result = udR_Failure_;

  if (source.empty())
  {
    spModel = pKernel->CreateComponent<UDModel>();
    if (spModel)
    {
      result = spModel->Load(name, useStreamer);

      if (result == udR_Success)
      {
        result = udOctree_GetLocalMatrixF64(spModel->GetOctreePtr(), udMat.a);
        if (result == udR_Success)
          source = name;
      }
    }
  }

  return (int)result;
}


udResult UDNode::Render(RenderSceneRef &spScene, const udDouble4x4 &mat)
{
  UDJob &job = spScene->ud.pushBack();
  memset(&job, 0, sizeof(job));

  job.renderModel.matrix = udMul(mat, udMat);
  job.renderModel.pOctree = spModel->GetOctreePtr();
  job.renderModel.pWorldMatrixD = job.renderModel.matrix.a;
  job.renderModel.pVoxelShader = pVoxelShader;
  job.renderModel.pPixelShader = pPixelShader;
  job.renderModel.flags = renderFlags;
  job.renderModel.startingRoot = startingRoot;

  if (pVoxelShader)
    job.renderModel.pVoxelShader = pVoxelShader;
  else if (simpleVoxelDel.GetMemento())
  {
    job.renderModel.pVoxelShader = NodeRenderModel::VoxelShaderFunc;
    job.renderModel.simpleVoxelDel = simpleVoxelDel;
  }


  if (clipAreaSet)
  {
    job.renderModel.clipArea = clipArea;
    job.renderModel.pClip = &job.renderModel.clipArea;
  }

  return udR_Success;
}


BoundingVolume UDNode::GetBoundingVolume() const
{
  BoundingVolume vol;

  UDASSERT(udMat.a[0] == udMat.a[5] && udMat.a[0] == udMat.a[10], "NonUniform Scale");

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

} // namespace ud
