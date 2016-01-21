#pragma once
#if !defined(EPRENDER_HPP)
#define EPRENDER_HPP

#include "ep/cpp/delegate.h"
#include "ep/cpp/math.h"
#include "ep/cpp/rect.h"

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

EP_ENUM(PrimType,
  Points,
  Lines,
  Triangles
);

struct RenderList
{
  PrimType type;
  size_t fistVert, firstIndex;
  size_t numPrimitives;
};

struct VertexArray
{
  ArrayBufferRef spArray;
  Array<SharedString, 1> attributes;
};

struct GeomRenderJob
{
  Double4x4 matrix;

  SharedArray<VertexArray> vertexArrays;
  ArrayBufferRef spIndices;
  MaterialRef spMaterial;

  Array<RenderList, 1> renderList;

  Delegate<void(SharedPtr<RefCounted>)> programCacheCallback;
  Delegate<void(SharedPtr<RefCounted>)> vertexFormatCacheCallback;
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
  using SimpleVoxelDlgt = Delegate<uint32_t(uint32_t color)>;

  Double4x4 matrix;

  SimpleVoxelDlgt simpleVoxelDel;

  uint32_t flags;
  uint32_t startingRoot;

  Rect<uint32_t> rect;
  bool useClip;
};

struct UDRenderJob
{
  UDModelRef spModel = nullptr;
  UDRenderState renderState;
};


class RenderScene
{
public:
  Array<GeomRenderJob, 16> geom;
  Array<UDRenderJob, 4> ud;
};

} // namespace ep

#endif // EPRENDER_HPP
