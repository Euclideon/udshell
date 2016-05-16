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

  uint32_t GetStartingRoot() const override final { return startingRoot; }
  void SetStartingRoot(uint32_t root) override final { startingRoot = root; }

  const Rect<uint32_t> &GetRenderClipRect() const override final { return rect; }
  void SetRenderClipRect(const Rect<uint32_t>& _rect) override final { rectSet = true; rect = _rect; }

  UDModelFlags GetRenderFlags() const override final { return renderFlags; }
  void SetRenderFlags(UDModelFlags flags) override final { renderFlags = flags; }

  const Double4x4 &GetUDMatrix() const override final { return udmatrix; }
  void SetUDMatrix(const Double4x4 &mat) override final { udmatrix = mat; }

  UDRenderState GetUDRenderState() const override final;

  VarDelegate GetVoxelVarDelegate() const override final { return voxelVarDelegate; }
  void SetVoxelVarDelegate(VarDelegate delegate) override final;

  int64_t GetMemoryUsage() const override final { return pOctree->memoryUsage; }

  EP_FRIENDS_WITH_IMPL(UDSource);
private:

  VarDelegate voxelVarDelegate;

  Double4x4 udmatrix;
  Rect<uint32_t> rect;
  udOctree *pOctree = nullptr;

  uint32_t startingRoot = 0;
  UDModelFlags renderFlags = UDModelFlags::None;
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
