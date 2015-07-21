#include "udUDNode.h"
#include "udOctree.h"

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
    "StartingRoot", // id
    "Starting Root", // displayName
    "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", // description
    &udUDNode::GetStartingRoot,
    &udUDNode::SetStartingRoot,
    udTypeDesc(udPropertyType::Integer)
  },

  {
    "RenderClipArea", // id
    "Render Clip Area", // displayName
    "Clipping Area of the Screen", // description
    &udUDNode::GetRenderClipArea,
    &udUDNode::SetRenderClipArea,
    udTypeDesc(udPropertyType::Struct)
  },

  {
    "RenderFlags", // id
    "Render Flags", // displayName
    "UD Rendering Flags", // description
    &udUDNode::GetRenderFlags,
    &udUDNode::SetRenderFlags,
    udTypeDesc(udPropertyType::Flags, 0, renderFlags)
  },
  {
    "DataSource", // id
    "udModel Data Source", // displayName
    "Data Source for UD Model", // description
    &udUDNode::GetSource,
    nullptr,
    udTypeDesc(udPropertyType::Flags, 0, renderFlags)
  }


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
    udTypeDesc(udPropertyType::Integer) // result
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
  [](){ return udR_Success; },             // pInitRender
  udUDNode::Create, // pCreateInstance

  udSlice<const udPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  udSlice<const udMethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};



int udUDNode::Load(udString name, bool useStreamer)
{
  udResult result;
  UD_ERROR_CHECK(udOctree_Create(&pOctree, name.toStringz(), useStreamer, 0));


epilogue:

  return (int)result;
}
