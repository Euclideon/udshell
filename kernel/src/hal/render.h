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

typedef void (PrimCallback)(size_t index, void *pUserData);

void epGPU_Clear(double color[4], double depth, int stencil);

void epGPU_RenderVertices(struct epShaderProgram *pProgram, struct epFormatDeclaration *pVertexDecl, struct epArrayBuffer *pVB[], epPrimitiveType primType, size_t vertexCount, size_t firstVertex = 0);
void epGPU_RenderIndices(struct epShaderProgram *pProgram, struct epFormatDeclaration *pVertexDecl, struct epArrayBuffer *pVB[], struct epArrayBuffer *pIB, epPrimitiveType primType, size_t indexCount, size_t firstIndex = 0, size_t firstVertex = 0);
void epGPU_RenderRanges(struct epShaderProgram *pProgram, struct epFormatDeclaration *pVertexDecl, struct epArrayBuffer *pVB[], epPrimitiveType primType, epVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback = nullptr, void *pCallbackData = nullptr);

// TODO: re-evaluate this
epSyncPoint *epGPU_CreateSyncPoint();
void epGPU_WaitSync(epSyncPoint **pSync);

#endif // EPGPU_H
