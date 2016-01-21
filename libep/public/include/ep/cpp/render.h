#pragma once
#if !defined(EPRENDER_HPP)
#define EPRENDER_HPP

#include "ep/cpp/delegate.h"
#include "ep/cpp/math.h"
#include "ep/cpp/rect.h"

// TODO: needs impl
//#include "ep/cpp/component/udmodel.h"
//#include "ep/cpp/component/array.h"
//#include "ep/cpp/component/shader.h"
//#include "ep/cpp/component/material.h"

namespace ep {

SHARED_CLASS(ArrayBuffer);
SHARED_CLASS(Shader);
SHARED_CLASS(UDModel);

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

struct GeomRenderJob
{
  Double4x4 matrix;

  uint32_t numTextures, numArrays;
#if 0 // TODO: For Manu
  ArrayRef textures[8];
  ArrayRef arrays[16];
  ArrayRef index;

  RenderShaderProgramRef spProgram;
  RenderVertexFormatRef spVertexFormat;

  BlendMode blendMode;
  CullMode cullMode;
#endif // 0
  // TODO: has stuff
  // constants
  // render states
};

class RenderScene
{
public:
  Array<UDRenderJob, 4> ud;
  Array<GeomRenderJob, 16> geom;
};

} // namespace ep

#endif // EPRENDER_HPP
