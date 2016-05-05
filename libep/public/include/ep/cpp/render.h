#pragma once
#if !defined(EPRENDER_HPP)
#define EPRENDER_HPP

#include "ep/cpp/delegate.h"
#include "ep/cpp/math.h"
#include "ep/cpp/rect.h"
#include "ep/cpp/internal/i/imaterial.h"

namespace ep {

SHARED_CLASS(ArrayBuffer);
SHARED_CLASS(Shader);
SHARED_CLASS(Material);
SHARED_CLASS(UDModel);

EP_ENUM(BlendMode,
  None,
  Alpha,
  Additive
);

EP_ENUM(CullMode,
  None,
  CW,
  CCW
);

EP_ENUM(CompareFunc,
  Never,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always,
  Disabled
);

EP_ENUM(StencilOp,
  Keep,
  Invert,
  Zero,
  Replace,
  Incr,
  IncrWrap,
  Decr,
  DecrWrap
);

EP_BITFIELD(MaterialFlags,
  Blending,
  Culling,
  DepthTest,
  StencilTest
);


EP_ENUM(PrimType,
  Points = 0,
  Lines,
  Triangles
);

struct StencilState
{
  StencilState(CompareFunc cmp = CompareFunc::Disabled, uint32_t val = 0, uint32_t msk = 0xFF,
               StencilOp st = StencilOp::Keep, StencilOp dp = StencilOp::Keep, StencilOp sc = StencilOp::Keep)
               : compare(cmp) , value(val), mask(msk), stencilFail(st), depthFail(dp), success(sc) {}

  CompareFunc compare;
  uint32_t value;
  uint32_t mask;
  StencilOp stencilFail;
  StencilOp depthFail;
  StencilOp success;
};

struct RenderShaderProperty
{
  Variant data;
  int index;
};

struct RenderList
{
  PrimType type;
  size_t firstVertex;
  size_t firstIndex;
  size_t numVertices;
};

struct GeomRenderJob
{
  Double4x4 matrix;

  MaterialRef spMaterial;

  SharedArray<ArrayBufferRef> vertexArrays;
  ArrayBufferRef spIndices;
  RenderList renderList;
  SharedPtr<RefCounted> spShaderInputConfig;

  Delegate<void(SharedPtr<RefCounted>)> retainShaderInputConfig;
};

EP_BITFIELD(UDRenderFlags,
  NoOrtho,
  RenderMask,
  RenderImmediate,
  RenderCellBounds,
  ClearTargets,
  ForceZCompare,
  QuickTest,
  PointCubes,
  DisablePatch,
  ForceSingleRoot,
  Patch2PixelOpt,
  Transparent,
  SynchronousStream
);

struct UDRenderState
{
  Double4x4 matrix;

  VarDelegate voxelVarDelegate;
  void *pVoxelVarDelegateMutex;

  uint32_t flags;
  uint32_t startingRoot;

  Rect<uint32_t> rect;
  bool useClip;
};

struct UDRenderJob
{
  UDModelRef spModel;
  UDRenderState renderState;
};


class RenderScene
{
public:
  Array<GeomRenderJob, 16> geom;
  Array<UDRenderJob, 4> ud;
};


inline Variant epToVariant(const StencilState &e)
{
  Variant::VarMap r;
  if (e.compare != CompareFunc::Disabled)
  {
    r.Insert("compare", e.compare);
    if (e.value != 0)
      r.Insert("value", e.value);
    if (e.mask != 0xFF)
      r.Insert("mask", e.mask);
    if (e.stencilFail != StencilOp::Keep)
      r.Insert("stencilfail", e.stencilFail);
    if (e.depthFail != StencilOp::Keep)
      r.Insert("depthfail", e.depthFail);
    if (e.success != StencilOp::Keep)
      r.Insert("success", e.success);
  }
  return std::move(r);
}

inline void epFromVariant(const Variant &v, StencilState *pE)
{
  *pE = StencilState();
  Variant *pV = v.getItem("compare");
  if (pV)
    pE->compare = pV->as<CompareFunc>();
  pV = v.getItem("value");
  if (pV)
    pE->value = pV->as<uint32_t>();
  pV = v.getItem("mask");
  if (pV)
    pE->mask = pV->as<uint32_t>();
  pV = v.getItem("stencilfail");
  if (pV)
    pE->stencilFail = pV->as<StencilOp>();
  pV = v.getItem("depthfail");
  if (pV)
    pE->depthFail = pV->as<StencilOp>();
  pV = v.getItem("success");
  if (pV)
    pE->success = pV->as<StencilOp>();
}

} // namespace ep

#endif // EPRENDER_HPP
