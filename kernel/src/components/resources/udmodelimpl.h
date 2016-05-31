#pragma once
#ifndef _EP_EPMODELIMPL_H
#define _EP_EPMODELIMPL_H

#include "ep/cpp/platform.h"
#include "udOctree.h"
#include "ep/cpp/render.h"
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

  uint32_t GetAnimationFrame() const override final { return animationFrame; }
  void SetAnimationFrame(uint32_t frame) override final { animationFrame = frame; }

  const Rect<uint32_t> &GetRenderClipRect() const override final { return rect; }
  void SetRenderClipRect(const Rect<uint32_t>& _rect) override final { rectSet = true; rect = _rect; }

  UDModelFlags GetRenderFlags() const override final { return renderFlags; }
  void SetRenderFlags(UDModelFlags flags) override final { renderFlags = flags; }

  const Double4x4 &GetUDMatrix() const override final { return udmatrix; }
  void SetUDMatrix(const Double4x4 &mat) override final { udmatrix = mat; }

  void SetVoxelFilter(VoxelFilter *pFilter) override final { pVoxelFilter = pFilter; }
  VoxelFilter* GetVoxelFilter() const override final { return pVoxelFilter; }

  void SetVoxelShader(VoxelShader *pShader) override final;
  VoxelShader* GetVoxelShader() const override final { return pVoxelShader; }

  void SetPixelShader(PixelShader *pShader) override final { pPixelShader = pShader; }
  PixelShader* GetPixelShader() const override final { return pPixelShader; }

  void SetConstantData(UDConstantDataType type, BufferRef buffer) override final { constantBuffers[type] = buffer; }
  BufferRef GetConstantData(UDConstantDataType type) const override final { return constantBuffers[type]; }

  VarDelegate GetVarVoxelShader() const override final { return varVoxelShader; }
  void SetVarVoxelShader(VarDelegate varShader) override final;

  void CopyRenderContext(UDRenderContext *pContext) const override final;
  int64_t GetMemoryUsage() const override final { return pOctree->memoryUsage; }

  EP_FRIENDS_WITH_IMPL(UDSource);
private:
  static uint32_t VoxelVarShaderFunc(udRenderModel *pRenderModel, udNodeIndex nodeIndex, udRenderNodeInfo *epUnusedParam(pNodeInfo));

  VarDelegate varVoxelShader;

  Double4x4 udmatrix;
  Rect<uint32_t> rect;
  udOctree *pOctree = nullptr;

  VoxelFilter *pVoxelFilter = nullptr;
  VoxelShader *pVoxelShader = nullptr;
  PixelShader *pPixelShader = nullptr;

  BufferRef constantBuffers[UDConstantDataType::PixelShader + 1];

  uint32_t startingRoot = 0;
  uint32_t animationFrame = 0;
  UDModelFlags renderFlags = UDModelFlags::None;

  bool rectSet = false;
};

inline void UDModelImpl::SetVoxelShader(VoxelShader *pShader)
{
  if (pShader)
    varVoxelShader = nullptr;

  pVoxelShader = pShader;
}

inline void UDModelImpl::SetVarVoxelShader(VarDelegate varShader)
{
  if (varShader)
    pVoxelShader = nullptr;

  varVoxelShader = varShader;
}

} // namespace ep

#endif // _EP_EPMODELIMPL_H
