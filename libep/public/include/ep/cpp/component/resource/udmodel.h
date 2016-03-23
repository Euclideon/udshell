#pragma once
#ifndef _EP_EPMODEL_H
#define _EP_EPMODEL_H

#include "ep/cpp/internal/i/iudmodel.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/delegate.h"

namespace ep {

SHARED_CLASS(UDModel);

class UDModel : public Resource, public IUDModel
{
  EP_DECLARE_COMPONENT_WITH_IMPL(UDModel, IUDModel, Resource, EPKERNEL_PLUGINVERSION, "UD model resource", 0)

public:
  uint32_t GetStartingRoot() const override final { return pImpl->GetStartingRoot(); }
  void SetStartingRoot(uint32_t root) override final { return pImpl->SetStartingRoot(root); }

  const Rect<uint32_t> &GetRenderClipRect() const override final { return pImpl->GetRenderClipRect(); }
  void SetRenderClipRect(const Rect<uint32_t>& _rect) override final { pImpl->SetRenderClipRect(_rect); }

  uint32_t GetRenderFlags() const override final { return pImpl->GetRenderFlags(); }
  void SetRenderFlags(uint32_t flags) override final { pImpl->SetRenderFlags(flags); }

  // This is temporary the Scale might be non uniform
  double GetUDScale() const override final { return pImpl->GetUDMatrix().a[0]; }
  const Double4x4 &GetUDMatrix() const override final { return pImpl->GetUDMatrix(); }

  BoundingVolume GetBoundingVolume() const override final { return pImpl->GetBoundingVolume(); }

  UDRenderState GetUDRenderState() const override final { return pImpl->GetUDRenderState(); }
  DataSourceRef GetDataSource() const override final { return pImpl->GetDataSource(); }

  VarDelegate GetVoxelVarDelegate() const override final { return pImpl->GetVoxelVarDelegate(); }
  void SetVoxelVarDelegate(VarDelegate del) override final { pImpl->SetVoxelVarDelegate(del); }

  EP_FRIENDS_WITH_IMPL(UDDataSource);
protected:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties();
};

} // namespace ep

#endif // _EP_EPMODEL_H
