#pragma once
#ifndef _EP_EPMODEL_H
#define _EP_EPMODEL_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "ep/cpp/internal/i/iudmodel.h"
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

#if 0 // Defined in iudmodel.h

EP_ENUM(UDConstantDataType,
  VoxelFilter,
  VoxelShader,
  PixelShader
);

enum UDFilterResult
{
  Fail = 0,                 // Fail the test, this implicitly won't test any children (eg completely outside)
  Pass = 1,                 // Pass and implicitly don't test any children (eg completely inside)
  PassAndTestChildren = 3,  // Pass but continue to test children (eg overlap inside/outside)

  UserMask = 0xffff0000     // The upper 16 bits can safely be used for user-defined state/flags
};
#endif


struct UDElementMetadata : public ElementMetadata
{
  UDAttributeBlend blend;
};

struct UDRenderNodeInfo
{
  size_t privateData;
  Double3 nodePosMS;           // The position of the minimum corner of the node, add childSizeMS to get center of the node
  double childSizeMS;          // Size of a child, or another way, HALF of the size of the node
  UDFilterResult filterResult; // The result of the filter function that is copied down to children and filtered again if udFR_TestChildren is set
  uint8_t level;
};

struct UDClipArea
{
  uint32_t minX;
  uint32_t minY;
  uint32_t maxX;
  uint32_t maxY;
};

// TODO: Expose this
struct UDView;

struct UDRender
{
  const Double4x4& GetMatrix() const;
  const UDClipArea& GetClipArea() const;
  UDModelFlags GetFlags() const;
  uint32_t GetStartingRoot() const;
  uint32_t GetAnimationFrame() const;
};

struct UDRenderNode : public UDRender
{
  uint32_t GetNodeColor(UDNodeIndex nodeIndex) const;
  template <typename T>
  const T& GetAttribute(UDNodeIndex nodeIndex, uint32_t attributeIndex) const;

  // Deprecated
  enum UDStreamType
  {
    Color = 1,
    Palette,
    UV,
    LightingComposite
  };
  template <typename T>
  const T& GetAttribute(UDNodeIndex nodeIndex, UDStreamType type) const;
};

struct UDFilterVoxel : public UDRenderNode
{
  template <typename T> const T& GetConstantData() const;
};

struct UDRenderVoxel : public UDRenderNode
{
  template <typename T> const T& GetConstantData() const;
};

struct UDRenderPixel : public UDRender
{
  template <typename T> const T& GetConstantData() const;
};

using VoxelFilter = UDFilterResult(const UDFilterVoxel &voxel, UDNodeIndex nodeIndex, const UDRenderNodeInfo &nodeInfo);
using VoxelShader = uint32_t(const UDRenderVoxel &voxel, UDNodeIndex nodeIndex, const UDRenderNodeInfo &nodeInfo);
using PixelShader = uint32_t(const UDRenderPixel &context, const UDView &view, const uint32_t &destColor, const float &destDepth, uint32_t srcColor, float srcDepth);

class UDModel : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, UDModel, IUDModel, Resource, EPKERNEL_PLUGINVERSION, "UD model resource", 0)
public:

  uint32_t GetStartingRoot() const { return pImpl->GetStartingRoot(); }
  void SetStartingRoot(uint32_t root) { return pImpl->SetStartingRoot(root); }

  uint32_t GetAnimationFrame() const { return pImpl->GetAnimationFrame(); }
  void SetAnimationFrame(uint32_t frame) { pImpl->SetAnimationFrame(frame); }

  const Rect<uint32_t> &GetRenderClipRect() const { return pImpl->GetRenderClipRect(); }
  void SetRenderClipRect(const Rect<uint32_t>& _rect) { pImpl->SetRenderClipRect(_rect); }

  UDModelFlags GetRenderFlags() const { return pImpl->GetRenderFlags(); }
  void SetRenderFlags(UDModelFlags flags) { pImpl->SetRenderFlags(flags); }

  const Double4x4 &GetUDMatrix() const { return pImpl->GetUDMatrix(); }
  void SetUDMatrix(const Double4x4 &matrix){ pImpl->SetUDMatrix(matrix); }

  void SetVoxelFilter(VoxelFilter *pVoxelFilter)  { pImpl->SetVoxelFilter(pVoxelFilter); }
  VoxelFilter* GetVoxelFilter() const { return pImpl->GetVoxelFilter(); }

  void SetVoxelShader(VoxelShader *pVoxelShader) { pImpl->SetVoxelShader(pVoxelShader); }
  VoxelShader* GetVoxelShader() const { return pImpl->GetVoxelShader(); }

  void SetPixelShader(PixelShader *pPixelShader) { pImpl->SetPixelShader(pPixelShader); }
  PixelShader* GetPixelShader() const { return pImpl->GetPixelShader(); }

  template <typename T>
  void SetConstantData(UDConstantDataType type, const T &data);

  template <typename T>
  void SetConstantData(UDConstantDataType type, Slice<const T> data);

  template <typename T>
  T GetConstantData(UDConstantDataType type);

  void SetConstantData(UDConstantDataType type, BufferRef buffer) { return pImpl->SetConstantData(type, buffer); }
  BufferRef GetConstantData(UDConstantDataType type) const { return pImpl->GetConstantData(type); }

  void SetVoxelFilterConstantData(BufferRef buffer) { SetConstantData(UDConstantDataType::VoxelFilter, buffer); }
  BufferRef GetVoxelFilterConstantData() const { return GetConstantData(UDConstantDataType::VoxelFilter); }

  void SetVoxelShaderConstantData(BufferRef buffer) { SetConstantData(UDConstantDataType::VoxelShader, buffer); }
  BufferRef GetVoxelShaderConstantData() const { return GetConstantData(UDConstantDataType::VoxelShader); }

  void SetPixelShaderConstantData(BufferRef buffer) { SetConstantData(UDConstantDataType::PixelShader, buffer); }
  BufferRef GetPixelShaderConstantData() const { return GetConstantData(UDConstantDataType::PixelShader); }

  int64_t GetMemoryUsage() const { return pImpl->GetMemoryUsage(); }

  // Deprecated
  VarDelegate GetVarVoxelShader() const { return pImpl->GetVarVoxelShader(); }
  void SetVarVoxelShader(VarDelegate del) { pImpl->SetVarVoxelShader(del); }

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

} // namespace ep

#include "ep/cpp/internal/udmodel_inl.h"

#endif // _EP_EPMODEL_H
