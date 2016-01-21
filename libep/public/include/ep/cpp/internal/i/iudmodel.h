#pragma once
#if !defined(_EP_IUDMODEL_HPP)
#define _EP_IUDMODEL_HPP

#include "ep/cpp/boundingvolume.h"
#include "ep/cpp/rect.h"
#include "ep/cpp/render.h"


namespace ep {

SHARED_CLASS(DataSource);

class IUDModel
{
public:
  virtual uint32_t GetStartingRoot() const = 0;
  virtual void SetStartingRoot(uint32_t root) = 0;

  virtual const Rect<uint32_t> &GetRenderClipRect() const = 0;
  virtual void SetRenderClipRect(const Rect<uint32_t>& _rect) = 0;

  virtual uint32_t GetRenderFlags() const = 0;
  virtual void SetRenderFlags(uint32_t flags) = 0;

  virtual double GetUDScale() const = 0;
  virtual const Double4x4 &GetUDMatrix() const = 0;
  virtual BoundingVolume GetBoundingVolume() const = 0;

  virtual UDRenderState GetUDRenderState() const = 0;
  virtual DataSourceRef GetDataSource() const = 0;

  // TODO: Revist the shader system.  Simple voxel shader is inadequate.
  typedef uint32_t SimpleVoxelDlgt(uint32_t color);
  virtual Delegate<SimpleVoxelDlgt> GetSimpleVoxelDelegate() const = 0;
  virtual void SetSimpleVoxelDelegate(Delegate<SimpleVoxelDlgt> del) = 0;

  //virtual udRender_VoxelShaderFunc *GetVoxelShader() const = 0;
  //virtual void SetVoxelShader(udRender_VoxelShaderFunc *pFunc) = 0;

  //virtual udRender_PixelShaderFunc *GetPixelShader() const = 0;
  //virtual void SetPixelShader(udRender_PixelShaderFunc *pFunc) = 0;

};

}

#endif // _EP_IUDMODEL_HPP
