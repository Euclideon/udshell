#pragma once
#ifndef UDPCNODE_H
#define UDPCNODE_H

#include "udComponent.h"
#include "udInput.h"
#include "udMath.h"
#include "udRender.h"
#include "udNode.h"
#include "udModel.h"

struct udOctree;

namespace udKernel
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

  SharedUDModelRef GetSharedUDModel() const { return  spModel;  } // TODO: Remove this once its no longer required by udglViewer

protected:
  UDNode(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Node(pType, pKernel, uid, initParams) { clipArea.minX = clipArea.minY = clipArea.maxX = clipArea.maxY = 0; }
  virtual ~UDNode() {}

  static Component *Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
  {
    return udNew(UDNode, pType, pKernel, uid, initParams);
  }

  udRender_VoxelShaderFunc *pVoxelShader = nullptr;
  udRender_PixelShaderFunc *pPixelShader = nullptr;
  udRenderClipArea clipArea;
  uint32_t startingRoot = 0;
  udRenderFlags renderFlags = udRF_None;
  bool clipAreaSet = false;

  udString source;
  SharedUDModelRef spModel = nullptr;
  udDouble4x4 udMat;
};


inline Variant ToVariant(const udRenderClipArea& area)
{
  KeyValuePair *pPairs = udAllocType(KeyValuePair, 4, udAF_None);
  if (pPairs)
  {
    udSlice<KeyValuePair> slice(pPairs, 4);

    new (&slice[0]) KeyValuePair("minx", area.minX);
    new (&slice[1]) KeyValuePair("miny", area.minY);
    new (&slice[2]) KeyValuePair("maxx", area.maxX);
    new (&slice[3]) KeyValuePair("maxy", area.maxX);

    return Variant(slice, true);
  }

  return Variant();
}

inline void udFromVariant(const Variant &variant, udRenderClipArea *pArea)
{
  pArea->minX = variant["minx"].as<uint32_t>();
  pArea->minY = variant["miny"].as<uint32_t>();
  pArea->maxX = variant["maxx"].as<uint32_t>();
  pArea->maxY = variant["maxy"].as<uint32_t>();
}


inline Variant ToVariant(const BoundingVolume &volume)
{
  KeyValuePair *pPairs = udAllocType(KeyValuePair, 6, udAF_None);
  if (pPairs)
  {
    udSlice<KeyValuePair> slice(pPairs, 6);

    new (&slice[0]) KeyValuePair("minx", volume.min.x);
    new (&slice[1]) KeyValuePair("miny", volume.min.y);
    new (&slice[0]) KeyValuePair("minz", volume.min.z);
    new (&slice[2]) KeyValuePair("maxx", volume.max.x);
    new (&slice[3]) KeyValuePair("maxy", volume.max.y);
    new (&slice[3]) KeyValuePair("maxz", volume.max.z);
    return Variant(slice, true);
  }

  return Variant();
}

inline void udFromVariant(const Variant &variant, BoundingVolume *pVolume)
{
  pVolume->min.x = variant["minx"].as<double>();
  pVolume->min.y = variant["miny"].as<double>();
  pVolume->min.z = variant["minz"].as<double>();
  pVolume->max.x = variant["maxx"].as<double>();
  pVolume->max.y = variant["maxy"].as<double>();
  pVolume->max.z = variant["maxz"].as<double>();
}

} // namespace udKernel

#endif // UDPCNODE_H
