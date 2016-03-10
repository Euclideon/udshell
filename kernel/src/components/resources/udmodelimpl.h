#pragma once
#ifndef _EP_EPMODELIMPL_H
#define _EP_EPMODELIMPL_H

#include "ep/cpp/platform.h"
#include "udOctree.h"

#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/boundingvolume.h"
#include "ep/cpp/rect.h"
#include "ep/cpp/kernel.h"
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

  uint32_t GetStartingRoot() const  override{ return startingRoot; }
  void SetStartingRoot(uint32_t root) override { startingRoot = root; }

  const Rect<uint32_t> &GetRenderClipRect() const override { return rect; }
  void SetRenderClipRect(const Rect<uint32_t>& _rect) override { rectSet = true; rect = _rect; }

  uint32_t GetRenderFlags() const override { return renderFlags; }
  void SetRenderFlags(uint32_t flags) override { renderFlags = (udRenderFlags)flags; }

  double GetUDScale() const override { EPASSERT(udmatrix.a[0] == udmatrix.a[5] && udmatrix.a[0] == udmatrix.a[10], "NonUniform Scale"); return udmatrix.a[0]; }
  const Double4x4 &GetUDMatrix() const override { return udmatrix; }

  BoundingVolume GetBoundingVolume() const override;
  UDRenderState GetUDRenderState() const override;

  DataSourceRef GetDataSource() const override { return wpDataSource; }

  VarDelegate GetVoxelVarDelegate() const override { return voxelVarDelegate; }
  void SetVoxelVarDelegate(VarDelegate delegate);

  EP_FRIENDS_WITH_IMPL(UDDataSource);
private:

  VarDelegate voxelVarDelegate;

  Double4x4 udmatrix;
  Rect<uint32_t> rect;
  SafePtr<DataSource> wpDataSource;
  udOctree *pOctree = nullptr;

  uint32_t startingRoot = 0;
  udRenderFlags renderFlags = udRF_None;
  bool rectSet = false;
};

inline void UDModelImpl::SetVoxelVarDelegate(VarDelegate delegate)
{
  if (delegate.GetMemento())
    voxelVarDelegate = delegate;
  else
    voxelVarDelegate = VarDelegate();

}

} // namespace ep

#endif // _EP_EPMODELIMPL_H
