#pragma once
#ifndef _EP_EPMODEL_H
#define _EP_EPMODEL_H

#include "ep/cpp/internal/i/iudmodel.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/delegate.h"

namespace ep {

SHARED_CLASS(UDModel);

EP_EXPLICIT_BITFIELD(UDModelFlags,
  None = 0,
  SynchronousStream = 4096,
  Transparent = 2048,
  DoNotWriteColor = 64,
  PixelApproxOpt = 1024
);

EP_ENUM(UDAttributeBlend,
  None = -1,
  Mean,
  NearestMean,
  Or,
  And,
  Min,
  Max,
  Sum,
  Product,
  Unique
  );

struct UDElementMetadata : public ElementMetadata
{
  UDAttributeBlend blend;
};

class UDModel : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, UDModel, IUDModel, Resource, EPKERNEL_PLUGINVERSION, "UD model resource", 0)

public:
  uint32_t GetStartingRoot() const { return pImpl->GetStartingRoot(); }
  void SetStartingRoot(uint32_t root) { return pImpl->SetStartingRoot(root); }

  const Rect<uint32_t> &GetRenderClipRect() const { return pImpl->GetRenderClipRect(); }
  void SetRenderClipRect(const Rect<uint32_t>& _rect) { pImpl->SetRenderClipRect(_rect); }

  UDModelFlags GetRenderFlags() const { return pImpl->GetRenderFlags(); }
  void SetRenderFlags(UDModelFlags flags) { pImpl->SetRenderFlags(flags); }

  // This is temporary the Scale might be non uniform
  double GetUDScale() const { return pImpl->GetUDMatrix().a[0]; }
  const Double4x4 &GetUDMatrix() const { return pImpl->GetUDMatrix(); }

  BoundingVolume GetBoundingVolume() const { return pImpl->GetBoundingVolume(); }

  UDRenderState GetUDRenderState() const { return pImpl->GetUDRenderState(); }

  VarDelegate GetVoxelVarDelegate() const { return pImpl->GetVoxelVarDelegate(); }
  void SetVoxelVarDelegate(VarDelegate del) { pImpl->SetVoxelVarDelegate(del); }

  EP_FRIENDS_WITH_IMPL(UDSource);
protected:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

private:
  Array<const PropertyInfo> GetProperties() const;
};

inline Variant epToVariant(const UDElementMetadata &e)
{
  Variant v = epToVariant(*static_cast<const ElementMetadata*>(&e));
  if (e.blend != UDAttributeBlend::None)
  {
    Variant::VarMap r = v.asAssocArray();
    r.Insert("blend", e.blend);
  }
  // v is a mutable reference to the VarMap
  return v;
}

inline void epFromVariant(const Variant &v, UDElementMetadata *pE)
{
  epFromVariant(v, static_cast<ElementMetadata*>(pE));
  Variant *pI = v.getItem("blend");
  if (pI)
    pE->blend = UDAttributeBlend(pI->as<int>());
  else
    pE->blend = UDAttributeBlend::None;
}

} // namespace ep

#endif // _EP_EPMODEL_H
