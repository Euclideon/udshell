#pragma once
#ifndef EPGPU_H
#define EPGPU_H

enum epPrimitiveType
{
  epPT_Points,
  epPT_Lines,
  epPT_LineStrip,
  epPT_Triangles,
  epPT_TriangleStrip,
  epPT_TriangleFan
};

struct epVertexRange
{
  uint32_t firstVertex;
  uint32_t vertexCount;
};

struct epSyncPoint;

enum epClear
{
  epC_Color = 1,
  epC_Depth = 2,
  epC_Stencil = 4
};

enum epCompareFunc
{
  epCF_Never,
  epCF_Less,
  epCF_Equal,
  epCF_LessEqual,
  epCF_Greater,
  epCF_NotEqual,
  epCF_GreaterEqual,
  epCF_Always
};

enum epFace
{
  epF_CW,
  epF_CCW,
  epF_Both
};

enum epBlendMode
{
  epBM_Alpha,
  epBM_Additive
};

typedef void (PrimCallback)(size_t index, void *pUserData);

void epGPU_Clear(uint32_t clearBits, float *pColor, float depth, int stencil);

void epGPU_RenderVertices(struct epShaderProgram *pProgram, struct epShaderInputConfig *pConfig, struct epArrayBuffer *pVB[], epPrimitiveType primType, size_t vertexCount, size_t firstVertex = 0);
void epGPU_RenderIndices(struct epShaderProgram *pProgram, struct epShaderInputConfig *pConfig, struct epArrayBuffer *pVB[], struct epArrayBuffer *pIB, epPrimitiveType primType, size_t indexCount, size_t firstIndex = 0, size_t firstVertex = 0);
void epGPU_RenderRanges(struct epShaderProgram *pProgram, struct epShaderInputConfig *pConfig, struct epArrayBuffer *pVB[], epPrimitiveType primType, epVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback = nullptr, void *pCallbackData = nullptr);

void epRenderState_SetCullMode(bool enable, epFace mode);

void epRenderState_SetDepthCompare(bool enable, epCompareFunc func);
void epRenderState_SetDepthMask(bool flag);

void epRenderState_SetBlendMode(bool enable, epBlendMode mode);
void epRenderState_SetColorMask(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// TODO: re-evaluate this
epSyncPoint *epGPU_CreateSyncPoint();
void epGPU_WaitSync(epSyncPoint **pSync);
void epGPU_DestroySyncPoint(epSyncPoint **ppSync);

#endif // EPGPU_H
