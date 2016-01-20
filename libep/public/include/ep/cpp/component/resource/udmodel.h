#pragma once
#ifndef _EP_EPMODEL_H
#define _EP_EPMODEL_H

#include "udPlatform.h"

#include "ep/cpp/internal/i/iudmodel.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/sharedptr.h"
#include "kernel.h"
#include "ep/cpp/delegate.h"

namespace ep {

SHARED_CLASS(UDModel);

class UDModel : public Resource, public IUDModel
{
  EP_DECLARE_COMPONENT_WITH_IMPL(UDModel, IUDModel, Resource, EPKERNEL_PLUGINVERSION, "UD model resource")

public:
  uint32_t GetStartingRoot() const override final { return pImpl->GetStartingRoot(); }
  void SetStartingRoot(uint32_t root) override final { return pImpl->SetStartingRoot(root); }

  const Rect<uint32_t> &GetRenderClipRect() const override final { return pImpl->GetRenderClipRect(); }
  void SetRenderClipRect(const Rect<uint32_t>& _rect) { pImpl->SetRenderClipRect(_rect); }

  uint32_t GetRenderFlags() const { return pImpl->GetRenderFlags(); }
  void SetRenderFlags(uint32_t flags) { pImpl->SetRenderFlags(flags); }

  // This is temporary the Scale might be non uniform
  double GetUDScale() const { return pImpl->GetUDMatrix().a[0]; }
  const Double4x4 &GetUDMatrix() const { return pImpl->GetUDMatrix(); }

  BoundingVolume GetBoundingVolume() const { return pImpl->GetBoundingVolume(); }

  UDRenderState GetUDRenderState() const { return pImpl->GetUDRenderState(); }
  DataSourceRef GetDataSource() const { return pImpl->GetDataSource(); }

  // TODO: Revist the shader system.  Simple voxel shader is inadequate.
  Delegate<SimpleVoxelDlgt> GetSimpleVoxelDelegate() const { return pImpl->GetSimpleVoxelDelegate(); }
  void SetSimpleVoxelDelegate(Delegate<SimpleVoxelDlgt> del) { pImpl->SetSimpleVoxelDelegate(del); }

  //udRender_VoxelShaderFunc *GetVoxelShader() const { return pImpl->GetVoxelShader(); }
  //void SetVoxelShader(udRender_VoxelShaderFunc *pFunc) { pImpl->SetVoxelShader(pFunc); }

  //udRender_PixelShaderFunc *GetPixelShader() const { return pImpl->GetPixelShader(); }
  //void SetPixelShader(udRender_PixelShaderFunc *pFunc) { pImpl->SetPixelShader(pFunc); }

  EP_FRIENDS_WITH_IMPL(UDDataSource);
protected:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

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
      // TODO: Can we uncomment this yet?
      //EP_MAKE_PROPERTY(VoxelShader, "Optional callback to handle writing pixels and depth", nullptr, 0),
    };
  }
};

} // namespace ep

#endif // _EP_EPMODEL_H
