#pragma once
#ifndef EP_UDNODE_H
#define EP_UDNODE_H

#include "udMath.h"
#include "udRender.h"

#include "ep/cpp/math.h"
#include "../component.h"
#include "node.h"
#include "components/resources/udmodel.h"
#include "hal/input.h"

struct udOctree;

namespace ep
{

struct BoundingVolume
{
  Double3 min;
  Double3 max;
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
  void SetVoxelShader(udRender_VoxelShaderFunc *pFunc) { simpleVoxelDel = Delegate<NodeRenderModel::SimpleVoxelDlgt>();  pVoxelShader = pFunc;  }

  Delegate<NodeRenderModel::SimpleVoxelDlgt> GetSimpleVoxelDelegate() const { return simpleVoxelDel; }
  void SetSimpleVoxelDelegate(Delegate<NodeRenderModel::SimpleVoxelDlgt> del);

  udRender_PixelShaderFunc *GetPixelShader() const { return pPixelShader;  }
  void SetPixelShader(udRender_PixelShaderFunc *pFunc)  { pPixelShader = pFunc; }

  uint32_t GetRenderFlags() const { return renderFlags; }
  void SetRenderFlags(uint32_t flags) { renderFlags = (udRenderFlags)flags; }

  String GetSource() const { return source;  }

  double GetUDScale() const { EPASSERT(udMat.a[0] == udMat.a[5] && udMat.a[0] == udMat.a[10], "NonUniform Scale"); return udMat.a[0]; }

  BoundingVolume GetBoundingVolume() const;

  int Load(String name, bool useStreamer); // TODO : Check return value to use error code

  UDModelRef GetUDModel() const { return  spModel;  } // TODO: Remove this once its no longer required by udglViewer

protected:
  UDNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Node(pType, pKernel, uid, initParams) { clipArea.minX = clipArea.minY = clipArea.maxX = clipArea.maxY = 0; }

  epResult Render(RenderSceneRef &spScene, const Double4x4 &mat) override;

  udRender_VoxelShaderFunc *pVoxelShader = nullptr;
  Delegate<NodeRenderModel::SimpleVoxelDlgt> simpleVoxelDel;

  udRender_PixelShaderFunc *pPixelShader = nullptr;

  udRenderClipArea clipArea;
  uint32_t startingRoot = 0;
  udRenderFlags renderFlags = udRF_None;
  bool clipAreaSet = false;

  String source;
  UDModelRef spModel = nullptr;
  Double4x4 udMat;
};


inline void UDNode::SetSimpleVoxelDelegate(Delegate<NodeRenderModel::SimpleVoxelDlgt> del)
{
  pVoxelShader = nullptr;
  if (del.GetMemento())
    simpleVoxelDel = del;
  else
    simpleVoxelDel = Delegate<NodeRenderModel::SimpleVoxelDlgt>();
}

Variant epToVariant(const BoundingVolume &volume);
void epFromVariant(const Variant &variant, BoundingVolume *pVolume);

} // namespace ep

Variant epToVariant(const udRenderClipArea& area);
void epFromVariant(const Variant &variant, udRenderClipArea *pArea);

#endif // EP_UDNODE_H
