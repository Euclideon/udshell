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

namespace kernel {

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
  EP_DECLARE_COMPONENT(UDModel, Resource, EPKERNEL_PLUGINVERSION, "UD model resource")
public:

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

  double GetUDScale() const { EPASSERT(udmatrix.a[0] == udmatrix.a[5] && udmatrix.a[0] == udmatrix.a[10], "NonUniform Scale"); return udmatrix.a[0]; }

  const Double4x4 &GetUDMatrix() const { return udmatrix; }

  BoundingVolume GetBoundingVolume() const;

  udOctree *GetOctreePtr() const { return pOctree; }

  UDRenderState GetUDRenderState() const;

  DataSourceRef GetDataSource() const { return spDataSource; }

  int Load(String name, bool useStreamer);

  friend class UDDataSource;
protected:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  virtual ~UDModel();

  udRender_VoxelShaderFunc *pVoxelShader = nullptr;
  Delegate<UDRenderState::SimpleVoxelDlgt> simpleVoxelDel;

  udRender_PixelShaderFunc *pPixelShader = nullptr;

  Double4x4 udmatrix;
  Rect rect;
  DataSourceRef spDataSource;
  udOctree *pOctree = nullptr;

  uint32_t startingRoot = 0;
  udRenderFlags renderFlags = udRF_None;
  bool rectSet = false;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(StartingRoot, "Normally zero, optionally set the starting root number (used with ForceSingleRoot flag)", nullptr, 0),
      EP_MAKE_PROPERTY(RenderClipRect, "Clipping Rect of the Screen", nullptr, 0),
      EP_MAKE_PROPERTY(RenderFlags, "UD Rendering Flags", nullptr, 0),
      EP_MAKE_PROPERTY_RO(DataSource, "Data Source for UD Model", nullptr, 0),
      EP_MAKE_PROPERTY_RO(UDScale, "Internal Scale of the Model", nullptr, 0),
      EP_MAKE_PROPERTY_RO(UDMatrix, "UD Matrix", nullptr, 0),
      EP_MAKE_PROPERTY_RO(BoundingVolume, "The Bouning Volume", nullptr, 0),
      EP_MAKE_PROPERTY(SimpleVoxelDelegate, "Optional callback to handle it's own internal call to GetNodeColor()", nullptr, 0),
//      EP_MAKE_PROPERTY(VoxelShader, "Optional callback to handle writing pixels and depth", nullptr, 0),
    };
  }
};

inline void UDModel::SetSimpleVoxelDelegate(Delegate<UDRenderState::SimpleVoxelDlgt> del)
{
  pVoxelShader = nullptr;
  if (del.GetMemento())
    simpleVoxelDel = del;
  else
    simpleVoxelDel = Delegate<UDRenderState::SimpleVoxelDlgt>();
}

} // namespace kernel

#endif // _EP_EPMODEL_H
