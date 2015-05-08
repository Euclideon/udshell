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

void udGPU_RenderVertices(struct udShaderProgram *pProgram, struct udVertexBuffer *pVB, udPrimitiveType primType, size_t numVertices);

#endif // UDGPU_H
