#pragma once
#ifndef UDPCNODE_H
#define UDPCNODE_H

#include "udMath.h"
#include "udRender.h"

#include "../component.h"
#include "node.h"
#include "resources/udmodel.h"
#include "hal/input.h"

struct udOctree;

namespace ud
{

struct BoundingVolume
{
  udDouble3 min;
  udDouble3 max;
};

class UDNode;
PROTOTYPE_COMPONENT(UDNode);


class UDNode : public Node
{
public:
  UD_COMPONENT(UDNode);

  udResult Render(RenderSceneRef &spScene, const udDouble4x4 &mat) override;

  uint32_t GetStartingRoot() const { return startingRoot; }
  void SetStartingRoot(uint32_t root) { startingRoot = root; }

  const udRenderClipArea& GetRenderClipArea() const { return clipArea; }
  void SetRenderClipArea(const udRenderClipArea& area) { clipAreaSet = true; clipArea = area; }

  udRender_VoxelShaderFunc *GetVoxelShader() const { return pVoxelShader;  }
  void SetVoxelShader(udRender_VoxelShaderFunc *pFunc) { pVoxelShader = pFunc;  }

  udRender_PixelShaderFunc *GetPixelShader() const { return pPixelShader;  }
  void SetPixelShader(udRender_PixelShaderFunc *pFunc)  { pPixelShader = pFunc; }

  uint32_t GetRenderFlags() const { return renderFlags; }
  void SetRenderFlags(uint32_t flags) { renderFlags = (udRenderFlags)flags; }

  udString GetSource() const { return source;  }

  double GetUDScale() const { UDASSERT(udMat.a[0] == udMat.a[5] && udMat.a[0] == udMat.a[10], "NonUniform Scale"); return udMat.a[0]; }

  BoundingVolume GetBoundingVolume() const;

  int Load(udString name, bool useStreamer); // TODO : Check return value to use error code

  UDModelRef GetSharedUDModel() const { return  spModel;  } // TODO: Remove this once its no longer required by udglViewer

protected:
  UDNode(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Node(pType, pKernel, uid, initParams) { clipArea.minX = clipArea.minY = clipArea.maxX = clipArea.maxY = 0; }
  virtual ~UDNode() {}

  udRender_VoxelShaderFunc *pVoxelShader = nullptr;
  udRender_PixelShaderFunc *pPixelShader = nullptr;
  udRenderClipArea clipArea;
  uint32_t startingRoot = 0;
  udRenderFlags renderFlags = udRF_None;
  bool clipAreaSet = false;

  udString source;
  UDModelRef spModel = nullptr;
  udDouble4x4 udMat;
};

inline udVariant udToVariant(const BoundingVolume &volume)
{
  udKeyValuePair *pPairs = udAllocType(udKeyValuePair, 6, udAF_None);
  if (pPairs)
  {
    udSlice<udKeyValuePair> slice(pPairs, 6);

    new (&slice[0]) udKeyValuePair("minx", volume.min.x);
    new (&slice[1]) udKeyValuePair("miny", volume.min.y);
    new (&slice[0]) udKeyValuePair("minz", volume.min.z);
    new (&slice[2]) udKeyValuePair("maxx", volume.max.x);
    new (&slice[3]) udKeyValuePair("maxy", volume.max.y);
    new (&slice[3]) udKeyValuePair("maxz", volume.max.z);
    return udVariant(slice, true);
  }

  return udVariant();
}

inline void udFromVariant(const udVariant &variant, BoundingVolume *pVolume)
{
  pVolume->min.x = variant["minx"].as<double>();
  pVolume->min.y = variant["miny"].as<double>();
  pVolume->min.z = variant["minz"].as<double>();
  pVolume->max.x = variant["maxx"].as<double>();
  pVolume->max.y = variant["maxy"].as<double>();
  pVolume->max.z = variant["maxz"].as<double>();
}

} // namespace ud


inline udVariant udToVariant(const udRenderClipArea& area)
{
  udKeyValuePair *pPairs = udAllocType(udKeyValuePair, 4, udAF_None);
  if (pPairs)
  {
    udSlice<udKeyValuePair> slice(pPairs, 4);

    new (&slice[0]) udKeyValuePair("minx", area.minX);
    new (&slice[1]) udKeyValuePair("miny", area.minY);
    new (&slice[2]) udKeyValuePair("maxx", area.maxX);
    new (&slice[3]) udKeyValuePair("maxy", area.maxX);

    return udVariant(slice, true);
  }

  return udVariant();
}

inline void udFromVariant(const udVariant &variant, udRenderClipArea *pArea)
{
  pArea->minX = variant["minx"].as<uint32_t>();
  pArea->minY = variant["miny"].as<uint32_t>();
  pArea->maxX = variant["maxx"].as<uint32_t>();
  pArea->maxY = variant["maxy"].as<uint32_t>();
}

#endif // UDPCNODE_H
