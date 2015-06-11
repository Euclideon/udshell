#pragma once
#ifndef UDGPU_H
#define UDGPU_H

enum udPrimitiveType
{
  udPT_Points,
  udPT_Lines,
  udPT_LineStrip,
  udPT_Triangles,
  udPT_TriangleStrip,
  udPT_TriangleFan
};

struct udVertexRange
{
  uint32_t firstVertex;
  uint32_t vertexCount;
};

typedef void (PrimCallback)(size_t index, void *pUserData);

void udGPU_RenderVertices(struct udShaderProgram *pProgram, struct udVertexBuffer *pVB, udPrimitiveType primType, size_t vertexCount, size_t firstVertex = 0);
void udGPU_RenderRanges(struct udShaderProgram *pProgram, struct udVertexBuffer *pVB, udPrimitiveType primType, udVertexRange *pRanges, size_t rangeCount, PrimCallback *pCallback = nullptr, void *pCallbackData = nullptr);

#endif // UDGPU_H
