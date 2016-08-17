#pragma once
#if !defined(EPRENDER_HPP)
#define EPRENDER_HPP

#include "ep/cpp/delegate.h"
#include "ep/cpp/math.h"
#include "ep/cpp/rect.h"
#include "ep/cpp/internal/i/imaterial.h"
#include "ep/cpp/internal/i/iudmodel.h"
#include "ep/cpp/component/node/node.h"

namespace ep {

SHARED_CLASS(ArrayBuffer);
SHARED_CLASS(Shader);
SHARED_CLASS(Material);
SHARED_CLASS(UDModel);

EP_ENUM(BlendMode,
  Alpha,
  Additive,
  None
);

EP_ENUM(CullMode,
  CW,
  CCW,
  Both,
  None
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

EP_ENUM(PrimType,
  Points = 0,
  Lines,
  Triangles
);

struct RenderShaderProperty
{
  Variant data;
  size_t index;
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

EP_EXPLICIT_BITFIELD(UDRenderFlags,
  None = 0,
  ClearTargets = 16,
  ForceZCompare = 32,
  DoNotWriteColor = 64,
  PixelApproxOpt = 1024,
  DoNotRenderCubes = 128,
  RenderPoints = 8192
);

struct UDRenderJob
{
  UDModelRef spModel;
  Double4x4 matrix;

  NodeRef udNodePtr; // TODO: Change this so there is no need to pass the scene ptr down
};

class RenderScene
{
public:
  Array<GeomRenderJob, 16> geom;
  Array<UDRenderJob, 4> ud;
};

} // namespace ep

#endif // EPRENDER_HPP
