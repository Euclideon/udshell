#pragma once
#ifndef _EP_EPMODEL_H
#define _EP_EPMODEL_H

#include "udPlatform.h"
#include "udOctree.h"

#include "components/resources/resource.h"
#include "ep/cpp/sharedptr.h"
#include "udRender.h"
#include "kernel.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/boundingvolume.h"
#include "ep/cpp/rect.h"

namespace ep {

struct UDRenderState : public udRenderModel
{
  typedef uint32_t(SimpleVoxelDlgt)(uint32_t color);

  Double4x4 matrix;
  udRenderClipArea clipArea;
  Delegate<SimpleVoxelDlgt> simpleVoxelDel;

  static unsigned VoxelShaderFunc(udRenderModel *pRenderModel, udNodeIndex nodeIndex, udRenderNodeInfo *epUnusedParam(pNodeInfo))
  {
    UDRenderState *pUDRenderState = static_cast<UDRenderState*>(pRenderModel);
    udOctree *pOctree = pRenderModel->pOctree;
    uint32_t color = pOctree->pGetNodeColor(pOctree, nodeIndex);
    // TODO : either wrap this in a critical section or create Lua states for each thread
    color = pUDRenderState->simpleVoxelDel(color);
    return color;
  }
};

PROTOTYPE_COMPONENT(UDModel);

class UDModel : public Resource
{
public:
  EP_COMPONENT(UDModel);

  uint32_t GetStartingRoot() const { return startingRoot; }
  void SetStartingRoot(uint32_t root) { startingRoot = root; }

  const Rect &GetRenderClipRect() const { return rect; }
  void SetRenderClipRect(const Rect& _rect) { rectSet = true; rect = _rect; }

  udRender_VoxelShaderFunc *GetVoxelShader() const { return pVoxelShader; }
  void SetVoxelShader(udRender_VoxelShaderFunc *pFunc) { simpleVoxelDel = Delegate<UDRenderState::SimpleVoxelDlgt>();  pVoxelShader = pFunc; }

  Delegate<UDRenderState::SimpleVoxelDlgt> GetSimpleVoxelDelegate() const { return simpleVoxelDel; }
  void SetSimpleVoxelDelegate(Delegate<UDRenderState::SimpleVoxelDlgt> del);

  udRender_PixelShaderFunc *GetPixelShader() const { return pPixelShader; }
  void SetPixelShader(udRender_PixelShaderFunc *pFunc)  { pPixelShader = pFunc; }

  uint32_t GetRenderFlags() const { return renderFlags; }
  void SetRenderFlags(uint32_t flags) { renderFlags = (udRenderFlags)flags; }

  double GetUDScale() const { EPASSERT(udMat.a[0] == udMat.a[5] && udMat.a[0] == udMat.a[10], "NonUniform Scale"); return udMat.a[0]; }

  BoundingVolume GetBoundingVolume() const;

  udOctree *GetOctreePtr() const { return pOctree; }

  UDRenderState GetUDRenderState() const;

  DataSourceRef GetDataSource() const { return spDataSource; }

  int Load(String name, bool useStreamer);

  friend class UDDataSource;
protected:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  virtual ~UDModel();

  udRender_VoxelShaderFunc *pVoxelShader = nullptr;
  Delegate<UDRenderState::SimpleVoxelDlgt> simpleVoxelDel;

  udRender_PixelShaderFunc *pPixelShader = nullptr;

  Double4x4 udMat;
  Rect rect;
  DataSourceRef spDataSource;
  udOctree *pOctree = nullptr;

  uint32_t startingRoot = 0;
  udRenderFlags renderFlags = udRF_None;
  bool rectSet = false;
};

inline void UDModel::SetSimpleVoxelDelegate(Delegate<UDRenderState::SimpleVoxelDlgt> del)
{
  pVoxelShader = nullptr;
  if (del.GetMemento())
    simpleVoxelDel = del;
  else
    simpleVoxelDel = Delegate<UDRenderState::SimpleVoxelDlgt>();
}

} // namespace ep

#endif // _EP_EPMODEL_H
