#pragma once
#if !defined(_EP_IUDMODEL_HPP)
#define _EP_IUDMODEL_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/boundingvolume.h"
#include "ep/cpp/rect.h"
#include "ep/cpp/render.h"

namespace ep {

SHARED_CLASS(DataSource);
SHARED_CLASS(Buffer);
EP_PROTOTYPE_ENUM(UDModelFlags);

struct UDRenderNodeInfo;
struct UDRender;
struct UDView;

enum UDFilterResult
{
  Fail = 0,                 // Fail the test, this implicitly won't test any children (eg completely outside)
  Pass = 1,                 // Pass and implicitly don't test any children (eg completely inside)
  PassAndTestChildren = 3,  // Pass but continue to test children (eg overlap inside/outside)

  UserMask = 0xffff0000     // The upper 16 bits can safely be used for user-defined state/flags
};

EP_ENUM(UDConstantDataType,
  VoxelFilter,
  VoxelShader,
  PixelShader
);


struct UDRenderPixel;
struct UDFilterVoxel;
struct UDRenderVoxel;


using UDNodeIndex = int64_t;

using IVoxelFilter = UDFilterResult(const UDFilterVoxel &voxel, UDNodeIndex nodeIndex, const UDRenderNodeInfo &nodeInfo);
using IVoxelShader = uint32_t(const UDRenderVoxel &voxel, UDNodeIndex nodeIndex, const UDRenderNodeInfo &nodeInfo);
using IPixelShader = uint32_t(const UDRenderPixel &context, const UDView &view, const uint32_t &destColor, const float &destDepth, uint32_t srcColor, float srcDepth);


namespace internal {
struct UDRenderPrivateData
{
  size_t private0[10] = { 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0 };
  uint32_t private1[3] = { 0, 0, 0 };
};

struct UDPrivateData
{
  uint8_t debugName[256];
  uint32_t private0[3];
  size_t private1[3];
  struct { uint32_t data[3]; } private2;
  size_t private3[11];
  volatile int32_t private4;
};

} // internal

struct UDRenderContext : public internal::UDRenderPrivateData
{
  struct
  {
    uint32_t x0, y0, x1, y1;
  } privateClip;

  Double4x4 matrix;
  VarDelegate varVoxelShader;
};

class IUDModel
{
public:
  virtual uint32_t GetStartingRoot() const = 0;
  virtual void SetStartingRoot(uint32_t root) = 0;

  virtual uint32_t GetAnimationFrame() const = 0;
  virtual void SetAnimationFrame(uint32_t frame) = 0;

  virtual const Rect<uint32_t> &GetRenderClipRect() const = 0;
  virtual void SetRenderClipRect(const Rect<uint32_t>& _rect) = 0;

  virtual UDModelFlags GetRenderFlags() const = 0;
  virtual void SetRenderFlags(UDModelFlags flags) = 0;

  virtual const Double4x4 &GetUDMatrix() const = 0;
  virtual void SetUDMatrix(const Double4x4 &matrix) = 0;

  virtual void SetVoxelFilter(IVoxelFilter *pVoxelShader) = 0;
  virtual IVoxelFilter* GetVoxelFilter() const = 0;

  virtual void SetVoxelShader(IVoxelShader *pVoxelShader) = 0;
  virtual IVoxelShader* GetVoxelShader() const = 0;

  virtual void SetPixelShader(IPixelShader *pPixelShader) = 0;
  virtual IPixelShader* GetPixelShader() const = 0;

  virtual VarDelegate GetVarVoxelShader() const = 0;
  virtual void SetVarVoxelShader(VarDelegate del) = 0;

  virtual void CopyRenderContext(UDRenderContext *pContext) const = 0;
  virtual int64_t GetMemoryUsage() const = 0;

  virtual void SetConstantData(UDConstantDataType type, BufferRef buffer) = 0;
  virtual BufferRef GetConstantData(UDConstantDataType type) const = 0;
};

}

#endif // _EP_IUDMODEL_HPP
