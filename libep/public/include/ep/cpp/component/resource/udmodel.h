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
// "octreeHeader" The octree header data (VarMap)
// "attributeInfo" Array of attribute descriptors (Array<UDElementMetadata>)

// "octreeHeader"
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

// "attributeInfo"
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
  const Double4x4& getMatrix() const;
  const UDClipArea& getClipArea() const;
  UDModelFlags getFlags() const;
  uint32_t getStartingRoot() const;
  uint32_t getAnimationFrame() const;
};

struct UDRenderNode : public UDRender
{
  uint32_t getNodeColor(UDNodeIndex nodeIndex) const;
  template <typename T>
  const T& getAttribute(UDNodeIndex nodeIndex, uint32_t attributeIndex) const;

  // Deprecated
  enum UDStreamType
  {
    Color = 1,
    Palette,
    UV,
    LightingComposite
  };
  template <typename T>
  const T& getAttribute(UDNodeIndex nodeIndex, UDStreamType type) const;
};

struct UDFilterVoxel : public UDRenderNode
{
  template <typename T> const T& getConstantData() const;
};

struct UDRenderVoxel : public UDRenderNode
{
  template <typename T> const T& getConstantData() const;
};

struct UDRenderPixel : public UDRender
{
  template <typename T> const T& getConstantData() const;
};

using VoxelFilter = UDFilterResult(const UDFilterVoxel &voxel, UDNodeIndex nodeIndex, const UDRenderNodeInfo &nodeInfo);
using VoxelShader = uint32_t(const UDRenderVoxel &voxel, UDNodeIndex nodeIndex, const UDRenderNodeInfo &nodeInfo);
using PixelShader = uint32_t(const UDRenderPixel &context, const UDView &view, const uint32_t &destColor, const float &destDepth, uint32_t srcColor, float srcDepth);

class UDModel : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, UDModel, IUDModel, Resource, EPKERNEL_PLUGINVERSION, "UD model resource", 0)
public:

  uint32_t getStartingRoot() const { return pImpl->GetStartingRoot(); }
  void setStartingRoot(uint32_t root) { return pImpl->SetStartingRoot(root); }

  uint32_t getAnimationFrame() const { return pImpl->GetAnimationFrame(); }
  void setAnimationFrame(uint32_t frame) { pImpl->SetAnimationFrame(frame); }

  const Rect<uint32_t> &getRenderClipRect() const { return pImpl->GetRenderClipRect(); }
  void setRenderClipRect(const Rect<uint32_t>& _rect) { pImpl->SetRenderClipRect(_rect); }

  UDModelFlags getRenderFlags() const { return pImpl->GetRenderFlags(); }
  void setRenderFlags(UDModelFlags flags) { pImpl->SetRenderFlags(flags); }

  const Double4x4 &getUDMatrix() const { return pImpl->GetUDMatrix(); }
  void setUDMatrix(const Double4x4 &matrix){ pImpl->SetUDMatrix(matrix); }

  void setVoxelFilter(VoxelFilter *pVoxelFilter)  { pImpl->SetVoxelFilter(pVoxelFilter); }
  VoxelFilter* getVoxelFilter() const { return pImpl->GetVoxelFilter(); }

  void setVoxelShader(VoxelShader *pVoxelShader) { pImpl->SetVoxelShader(pVoxelShader); }
  VoxelShader* getVoxelShader() const { return pImpl->GetVoxelShader(); }

  void setPixelShader(PixelShader *pPixelShader) { pImpl->SetPixelShader(pPixelShader); }
  PixelShader* getPixelShader() const { return pImpl->GetPixelShader(); }

  template <typename T>
  void setConstantData(UDConstantDataType type, const T &data);

  template <typename T>
  void setConstantData(UDConstantDataType type, Slice<const T> data);

  template <typename T>
  T getConstantData(UDConstantDataType type);

  void setConstantData(UDConstantDataType type, BufferRef buffer) { return pImpl->SetConstantData(type, buffer); }
  BufferRef getConstantData(UDConstantDataType type) const { return pImpl->GetConstantData(type); }

  void setVoxelFilterConstantData(BufferRef buffer) { setConstantData(UDConstantDataType::VoxelFilter, buffer); }
  BufferRef getVoxelFilterConstantData() const { return getConstantData(UDConstantDataType::VoxelFilter); }

  void setVoxelShaderConstantData(BufferRef buffer) { setConstantData(UDConstantDataType::VoxelShader, buffer); }
  BufferRef getVoxelShaderConstantData() const { return getConstantData(UDConstantDataType::VoxelShader); }

  void setPixelShaderConstantData(BufferRef buffer) { setConstantData(UDConstantDataType::PixelShader, buffer); }
  BufferRef getPixelShaderConstantData() const { return getConstantData(UDConstantDataType::PixelShader); }

  int64_t getMemoryUsage() const { return pImpl->GetMemoryUsage(); }

  // Deprecated
  VarDelegate getVarVoxelShader() const { return pImpl->GetVarVoxelShader(); }
  void setVarVoxelShader(VarDelegate del) { pImpl->SetVarVoxelShader(del); }

  EP_FRIENDS_WITH_IMPL(UDSource);
protected:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

private:
  Array<const PropertyInfo> getProperties() const;

};

} // namespace ep

#include "ep/cpp/internal/udmodel_inl.h"

#endif // _EP_EPMODEL_H
