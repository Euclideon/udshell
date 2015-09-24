#pragma once
#ifndef EPPCNODE_H
#define EPPCNODE_H

#include "udMath.h"
#include "udRender.h"

#include "../component.h"
#include "node.h"
#include "components/resources/udmodel.h"
#include "hal/input.h"

struct udOctree;

namespace ep
{

struct BoundingVolume
{
  udDouble3 min;
  udDouble3 max;
};

PROTOTYPE_COMPONENT(UDNode);

class UDNode : public Node
{
public:
  EP_COMPONENT(UDNode);

  uint32_t GetStartingRoot() const { return startingRoot; }
  void SetStartingRoot(uint32_t root) { startingRoot = root; }

  const udRenderClipArea& GetRenderClipArea() const { return clipArea; }
  void SetRenderClipArea(const udRenderClipArea& area) { clipAreaSet = true; clipArea = area; }

  udRender_VoxelShaderFunc *GetVoxelShader() const { return pVoxelShader;  }
  void SetVoxelShader(udRender_VoxelShaderFunc *pFunc) { simpleVoxelDel = epDelegate<NodeRenderModel::SimpleVoxelDlgt>();  pVoxelShader = pFunc;  }

  epDelegate<NodeRenderModel::SimpleVoxelDlgt> GetSimpleVoxelDelegate() const { return simpleVoxelDel; }
  void SetSimpleVoxelDelegate(epDelegate<NodeRenderModel::SimpleVoxelDlgt> del);

  udRender_PixelShaderFunc *GetPixelShader() const { return pPixelShader;  }
  void SetPixelShader(udRender_PixelShaderFunc *pFunc)  { pPixelShader = pFunc; }

  uint32_t GetRenderFlags() const { return renderFlags; }
  void SetRenderFlags(uint32_t flags) { renderFlags = (udRenderFlags)flags; }

  epString GetSource() const { return source;  }

  double GetUDScale() const { EPASSERT(udMat.a[0] == udMat.a[5] && udMat.a[0] == udMat.a[10], "NonUniform Scale"); return udMat.a[0]; }

  BoundingVolume GetBoundingVolume() const;

  int Load(epString name, bool useStreamer); // TODO : Check return value to use error code

  UDModelRef GetUDModel() const { return  spModel;  } // TODO: Remove this once its no longer required by udglViewer

protected:
  UDNode(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Node(pType, pKernel, uid, initParams) { clipArea.minX = clipArea.minY = clipArea.maxX = clipArea.maxY = 0; }

  udResult Render(RenderSceneRef &spScene, const udDouble4x4 &mat) override;

  udRender_VoxelShaderFunc *pVoxelShader = nullptr;
  epDelegate<NodeRenderModel::SimpleVoxelDlgt> simpleVoxelDel;

  udRender_PixelShaderFunc *pPixelShader = nullptr;

  udRenderClipArea clipArea;
  uint32_t startingRoot = 0;
  udRenderFlags renderFlags = udRF_None;
  bool clipAreaSet = false;

  epString source;
  UDModelRef spModel = nullptr;
  udDouble4x4 udMat;
};


inline void UDNode::SetSimpleVoxelDelegate(epDelegate<NodeRenderModel::SimpleVoxelDlgt> del)
{
  pVoxelShader = nullptr;
  if (del.GetMemento())
    simpleVoxelDel = del;
  else
    simpleVoxelDel = epDelegate<NodeRenderModel::SimpleVoxelDlgt>();
}

inline epVariant epToVariant(const BoundingVolume &volume)
{
  epKeyValuePair *pPairs = udAllocType(epKeyValuePair, 6, udAF_None);
  if (pPairs)
  {
    epSlice<epKeyValuePair> slice(pPairs, 6);

    new (&slice[0]) epKeyValuePair("minx", volume.min.x);
    new (&slice[1]) epKeyValuePair("miny", volume.min.y);
    new (&slice[2]) epKeyValuePair("minz", volume.min.z);
    new (&slice[3]) epKeyValuePair("maxx", volume.max.x);
    new (&slice[4]) epKeyValuePair("maxy", volume.max.y);
    new (&slice[5]) epKeyValuePair("maxz", volume.max.z);
    return epVariant(slice, true);
  }

  return epVariant();
}

inline void epFromVariant(const epVariant &variant, BoundingVolume *pVolume)
{
  pVolume->min.x = variant["minx"].as<double>();
  pVolume->min.y = variant["miny"].as<double>();
  pVolume->min.z = variant["minz"].as<double>();
  pVolume->max.x = variant["maxx"].as<double>();
  pVolume->max.y = variant["maxy"].as<double>();
  pVolume->max.z = variant["maxz"].as<double>();
}

} // namespace ep


inline epVariant epToVariant(const udRenderClipArea& area)
{
  epKeyValuePair *pPairs = udAllocType(epKeyValuePair, 4, udAF_None);
  if (pPairs)
  {
    epSlice<epKeyValuePair> slice(pPairs, 4);

    new (&slice[0]) epKeyValuePair("minx", area.minX);
    new (&slice[1]) epKeyValuePair("miny", area.minY);
    new (&slice[2]) epKeyValuePair("maxx", area.maxX);
    new (&slice[3]) epKeyValuePair("maxy", area.maxX);

    return epVariant(slice, true);
  }

  return epVariant();
}

inline void epFromVariant(const epVariant &variant, udRenderClipArea *pArea)
{
  pArea->minX = variant["minx"].as<uint32_t>();
  pArea->minY = variant["miny"].as<uint32_t>();
  pArea->maxX = variant["maxx"].as<uint32_t>();
  pArea->maxY = variant["maxy"].as<uint32_t>();
}

#endif // EPPCNODE_H
