#pragma once
#ifndef UDPCNODE_H
#define UDPCNODE_H

#include "udComponent.h"
#include "udInput.h"
#include "udMath.h"
#include "udRender.h"
#include "udNode.h"

class udUDNode;
PROTOTYPE_COMPONENT(udUDNode);

struct udOctree;

class udUDNode : public udNode
{
public:
  UD_COMPONENT(udUDNode);

  uint32_t GetStartingRoot() const { return startingRoot; }
  void SetStartingRoot(uint32_t root) { startingRoot = root; }

  const udRenderClipArea& GetRenderClipArea() const { return clipArea; }
  void SetRenderClipArea(const udRenderClipArea& area) { clipArea = area; }

  udRender_VoxelShaderFunc *GetVoxelShader() const { return pVoxelShader;  }
  void SetVoxelShader(udRender_VoxelShaderFunc *pFunc) { pVoxelShader = pFunc;  }

  udRender_PixelShaderFunc *GetPixelShader() const { return pPixelShader;  }
  void SetPixelShader(udRender_PixelShaderFunc *pFunc)  { pPixelShader = pFunc; }

  uint32_t GetRenderFlags() const { return renderFlags; }
  void SetRenderFlags(uint32_t flags) { renderFlags = (udRenderFlags)flags; }

  udString GetSource() const; // This will become a udComponetSource

  int Load(udString name, bool useStreamer); // TODO : Check return value to use error code

protected:
  udUDNode(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
    : udNode(pType, pKernel, uid, initParams) {}
  virtual ~udUDNode() {}

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udUDNode, pType, pKernel, uid, initParams);
  }

  udRender_VoxelShaderFunc *pVoxelShader;
  udRender_PixelShaderFunc *pPixelShader;
  udRenderClipArea clipArea;
  uint32_t startingRoot;
  udRenderFlags renderFlags;

  udString source;
  udOctree *pOctree;
};


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
