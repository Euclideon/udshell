#pragma once
#ifndef _EP_EPMODELIMPL_H
#define _EP_EPMODELIMPL_H

#include "udPlatform.h"
#include "udOctree.h"

#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/boundingvolume.h"
#include "ep/cpp/rect.h"
#include "kernel.h"
#include "udRender.h"


namespace ep {

SHARED_CLASS(UDModel);

class UDModelImpl : public BaseImpl<UDModel, IUDModel>
{
public:
  friend class SceneImpl;
  UDModelImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance) {}
  virtual ~UDModelImpl();

  uint32_t GetStartingRoot() const { return startingRoot; }
  void SetStartingRoot(uint32_t root) { startingRoot = root; }

  const Rect<uint32_t> &GetRenderClipRect() const { return rect; }
  void SetRenderClipRect(const Rect<uint32_t>& _rect) { rectSet = true; rect = _rect; }

  uint32_t GetRenderFlags() const { return renderFlags; }
  void SetRenderFlags(uint32_t flags) { renderFlags = (udRenderFlags)flags; }

  double GetUDScale() const { EPASSERT(udmatrix.a[0] == udmatrix.a[5] && udmatrix.a[0] == udmatrix.a[10], "NonUniform Scale"); return udmatrix.a[0]; }
  const Double4x4 &GetUDMatrix() const { return udmatrix; }

  BoundingVolume GetBoundingVolume() const;
  UDRenderState GetUDRenderState() const;

  DataSourceRef GetDataSource() const { return wpDataSource; }

  // TODO: Revist the shader system.  Simple voxel shader is inadequate.
  Delegate<SimpleVoxelDlgt> GetSimpleVoxelDelegate() const { return simpleVoxelDel; }
  void SetSimpleVoxelDelegate(Delegate<SimpleVoxelDlgt> del);

  //udRender_VoxelShaderFunc *GetVoxelShader() const { return pVoxelShader; }
  //void SetVoxelShader(udRender_VoxelShaderFunc *pFunc) { simpleVoxelDel = Delegate<UDRenderState::SimpleVoxelDlgt>();  pVoxelShader = pFunc; }

  //udRender_PixelShaderFunc *GetPixelShader() const { return pPixelShader; }
  //void SetPixelShader(udRender_PixelShaderFunc *pFunc) { pPixelShader = pFunc; }

  EP_FRIENDS_WITH_IMPL(UDDataSource);
private:

  Delegate<SimpleVoxelDlgt> simpleVoxelDel;

  //udRender_VoxelShaderFunc *pVoxelShader = nullptr;
  //udRender_PixelShaderFunc *pPixelShader = nullptr;

  Double4x4 udmatrix;
  Rect<uint32_t> rect;
  SafePtr<DataSource> wpDataSource;
  udOctree *pOctree = nullptr;

  uint32_t startingRoot = 0;
  udRenderFlags renderFlags = udRF_None;
  bool rectSet = false;
};

inline void UDModelImpl::SetSimpleVoxelDelegate(Delegate<SimpleVoxelDlgt> del)
{
  //pVoxelShader = nullptr;
  if (del.GetMemento())
    simpleVoxelDel = del;
  else
    simpleVoxelDel = Delegate<SimpleVoxelDlgt>();
}

} // namespace ep

#endif // _EP_EPMODELIMPL_H
