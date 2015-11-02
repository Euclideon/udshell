#include "udnode.h"
#include "udOctree.h"
#include "renderscene.h"
#include "kernel.h"
#include "udRender.h"
#include "ep/cpp/delegate.h"
#include "components/datasources/uddatasource.h"

namespace ep
{

static const Slice<const EnumKVP> renderFlags =
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

static CPropertyDesc props[] =
{
  {
    {
      "startingroot", // id
      "Starting Root", // displayName
      "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", // description
    },
    &UDNode::GetStartingRoot,
    &UDNode::SetStartingRoot
  },

  {
    {
      "rendercliparea", // id
      "Render Clip Area", // displayName
      "Clipping Area of the Screen", // description
    },
    &UDNode::GetRenderClipArea,
    &UDNode::SetRenderClipArea
  },

  {
    {
      "renderflags", // id
      "Render Flags", // displayName
      "UD Rendering Flags", // description
    },
    &UDNode::GetRenderFlags,
    &UDNode::SetRenderFlags
  },
  {
    {
      "datasource", // id
      "udModel Data Source", // displayName
      "Data Source for UD Model", // description
    },
    &UDNode::GetSource,
    nullptr,
  },

  {
    {
      "udscale", // id
      "UD Scale", // displayName
      "Internal Scale of the Model", // description
    },
    &UDNode::GetUDScale,
    nullptr
  },

  {
    {
      "boundingvolume", // id
      "Bounding Volume", // displayName
      "The Bouning Volume", // description
    },
    &UDNode::GetBoundingVolume,
    nullptr
  },


  {
    {
      "voxelshader", // id
      "Voxel Shader", // displayName
      "Optional callback to handle it's own internal call to GetNodeColor()", // description
    },
    &UDNode::GetSimpleVoxelDelegate,
    &UDNode::SetSimpleVoxelDelegate
  },
#if 0
  {
    {
      "PixelShader", // id
      "Pixel Shader", // displayName
      "Optional callback to handle writing pixels and depth", // description
    },
    nullptr, //Getter(&UDNode::GetVoxelShader),
    Setter(&UDNode::SetVoxelShader)
}

#endif
};

static CMethodDesc methods[] =
{
  {
    {
      "load",
      "Load the UD Model",
    },
    &UDNode::Load // method
  }
};


ComponentDesc UDNode::descriptor =
{
  &Node::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "udnode",    // id
  "UDNode",  // displayName
  "Is a UD model node", // description

  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)), // properties
  Slice<CMethodDesc>(methods, UDARRAYSIZE(methods)) // methods
};


int UDNode::Load(String name, bool useStreamer)
{
  udResult result = udR_Failure_;
  if (source.empty())
  {
    UDDataSourceRef udSource = pKernel->CreateComponent<UDDataSource>({ { "src", name }, {"useStreamer", useStreamer } });
    if (udSource)
    {
      spModel = udSource->GetResourceAs<UDModel>(name);
      if (spModel)
      {
         result = udOctree_GetLocalMatrixF64(spModel->GetOctreePtr(), udMat.a);
         if (result == udR_Success)
           source = name;
      }
    }
  }

  return (int)result;
}


epResult UDNode::Render(RenderSceneRef &spScene, const Double4x4 &mat)
{
  UDJob &job = spScene->ud.pushBack();
  memset(&job, 0, sizeof(job));

  (Double4x4&)job.renderModel.matrix = Mul(mat, udMat);
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

  return epR_Success;
}


BoundingVolume UDNode::GetBoundingVolume() const
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

Variant epToVariant(const BoundingVolume &volume)
{
  Array<KeyValuePair> kvp;
  kvp.reserve(6);
  kvp.pushBack(KeyValuePair("minx", volume.min.x));
  kvp.pushBack(KeyValuePair("miny", volume.min.y));
  kvp.pushBack(KeyValuePair("minz", volume.min.z));
  kvp.pushBack(KeyValuePair("maxx", volume.max.x));
  kvp.pushBack(KeyValuePair("maxy", volume.max.y));
  kvp.pushBack(KeyValuePair("maxz", volume.max.z));
  return std::move(kvp);
}

void epFromVariant(const Variant &variant, BoundingVolume *pVolume)
{
  pVolume->min.x = variant["minx"].as<double>();
  pVolume->min.y = variant["miny"].as<double>();
  pVolume->min.z = variant["minz"].as<double>();
  pVolume->max.x = variant["maxx"].as<double>();
  pVolume->max.y = variant["maxy"].as<double>();
  pVolume->max.z = variant["maxz"].as<double>();
}

} // namespace ep

Variant epToVariant(const udRenderClipArea& area)
{
  Array<KeyValuePair> kvp;
  kvp.reserve(4);
  kvp.pushBack(KeyValuePair("minx", area.minX));
  kvp.pushBack(KeyValuePair("miny", area.minY));
  kvp.pushBack(KeyValuePair("maxx", area.maxX));
  kvp.pushBack(KeyValuePair("maxy", area.maxX));
  return std::move(kvp);
}

void epFromVariant(const Variant &variant, udRenderClipArea *pArea)
{
  pArea->minX = variant["minx"].as<uint32_t>();
  pArea->minY = variant["miny"].as<uint32_t>();
  pArea->maxX = variant["maxx"].as<uint32_t>();
  pArea->maxY = variant["maxy"].as<uint32_t>();
}
