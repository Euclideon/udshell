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

// UDModel Metadata
// ----------------
// "octreeheader" The octree header data (VarMap)
// "attributeinfo" Array of attribute descriptors (Array<UDElementMetadata>)

// "octreeheader"
//  This is a VarMap containg the models header data.
//   Key              Type
//  ------------------------
//  "scale"           double
//  "unitMeterScale   double
//  "pivotOrigin"     Double3
//  "sourceBias"      Double3
//  "sourceScale"     Double3
//  "boundingVolume"  BoundingVolume
//  "maxOctreeDepth"  uint32_t

// "attributeinfo"
//  This is an array of UDElementMetadatas. The attributes present in each node.

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

  const Double4x4 &GetUDMatrix() const { return pImpl->GetUDMatrix(); }
  void SetUDMatrix(const Double4x4 &matrix){ pImpl->SetUDMatrix(matrix); }

  UDRenderState GetUDRenderState() const { return pImpl->GetUDRenderState(); }

  VarDelegate GetVoxelVarDelegate() const { return pImpl->GetVoxelVarDelegate(); }
  void SetVoxelVarDelegate(VarDelegate del) { pImpl->SetVoxelVarDelegate(del); }

  int64_t GetMemoryUsage() const { return pImpl->GetMemoryUsage(); }

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
