#include "components/primitivegenerator.h"
#include "ep/cpp/component/resource/model.h"
#include "ep/cpp/component/resource/arraybuffer.h"

namespace ep {

Array<const MethodInfo> PrimitiveGenerator::GetMethods() const
{
  return{
    EP_MAKE_METHOD(GenerateCube, "Generate a cube primitive"),
    EP_MAKE_METHOD(GenerateSphere, "Generate a sphere primitive"),
    EP_MAKE_METHOD(GenerateCylinder, "Generate a cylinder primitive"),
    EP_MAKE_METHOD(GenerateCone, "Generate a cone primitive"),
  };
}

void PrimitiveGenerator::GenerateCube(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex)
{
  // vertex buffer
  spVB->Allocate<Float3>(8);
  Slice<Float3> vb = spVB->Map<Float3>();
  epscope(exit) { spVB->Unmap(); };

  vb[0] = Float3{-.5, .5, .5 };
  vb[1] = Float3{ .5, .5, .5 };
  vb[2] = Float3{-.5,-.5, .5 };
  vb[3] = Float3{ .5,-.5, .5 };
  vb[4] = Float3{-.5, .5,-.5 };
  vb[5] = Float3{ .5, .5,-.5 };
  vb[6] = Float3{-.5,-.5,-.5 };
  vb[7] = Float3{ .5,-.5,-.5 };

  if (transformVertex)
  {
    for (Float3 &v : vb)
      v = transformVertex(v);
  }

  // index buffer
  spIB->AllocateFromData<uint16_t>(Slice<const uint16_t>
  {
    0,1,2, 1,3,2, // top
    2,3,6, 3,7,6, // front
    3,1,7, 1,5,7, // right
    1,0,5, 0,4,5, // back
    0,2,4, 2,6,4, // left
    7,6,4, 7,4,5  // bottom
  });
}

void PrimitiveGenerator::GenerateSphere(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex)
{
  EPASSERT_THROW(numSegments >= 3 && numSlices >= 2, epR_InvalidArgument, "Invalid args!");

  size_t numVerts = 2 + numSegments*(numSlices-1);
  size_t numIndices = numSegments*(numSlices-1)*2*3;

  spVB->Allocate<Float3>(numVerts);
  spIB->Allocate<uint16_t>(numIndices);

  Slice<Float3> vb = spVB->Map<Float3>();
  epscope(exit) { spVB->Unmap(); };
  Slice<uint16_t> ib = spIB->Map<uint16_t>();
  epscope(exit) { spIB->Unmap(); };

  size_t v = 0;
  size_t i = 0;

  // top and bottom center verts
  vb[v++] = Float3{ 0.0, 0.0, 1.0 };
  vb[numVerts - 1] = Float3{ 0.0, 0.0, 0.0 };

  for (int s = 1; s < numSlices; ++s)
  {
    // calculate z and ring-scale
    double ratio = s/double(numSlices);
    double radius = Sin(EP_PI*ratio)*0.5;
    float z = (float)(Cos(EP_PI*ratio)*0.5 + 0.5);

    for (int j = 0; j < numSegments + 1; ++j)
    {
      // insert triangles
      if (j > 0)
      {
        size_t wrap = j == numSegments ? numSegments : 0;
        if (s == 1)
        {
          // top cap
          ib[i++] = (uint16_t)(0);
          ib[i++] = (uint16_t)(v - 1);
          ib[i++] = (uint16_t)(v - wrap);
        }
        else
        {
          // central surface
          ib[i++] = (uint16_t)(v - wrap - numSegments);
          ib[i++] = (uint16_t)(v - numSegments - 1);
          ib[i++] = (uint16_t)(v - wrap);
          ib[i++] = (uint16_t)(v - numSegments - 1);
          ib[i++] = (uint16_t)(v - 1);
          ib[i++] = (uint16_t)(v - wrap);
        }
        if (s == numSlices - 1)
        {
          ib[i++] = (uint16_t)(numVerts - 1);
          ib[i++] = (uint16_t)(v - wrap);
          ib[i++] = (uint16_t)(v - 1);
        }
      }

      // insert the vertex
      if (j < numSegments)
      {
        // add vertex
        double a = EP_2PI*double(j) / double(numSegments);
        vb[v++] = Float3{ (float)(Sin(a)*radius), (float)(Cos(a)*radius), (float)z };
      }
    }
  }

  EPASSERT(++v == numVerts, "Incorrect number of vertices!");
  EPASSERT(i == numIndices, "Incorrect number of indices!");

  if (transformVertex)
  {
    for (Float3 &vert : vb)
      vert = transformVertex(vert);
  }
}

void PrimitiveGenerator::GenerateCylinder(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex)
{
  EPASSERT_THROW(numSegments >= 3 && numSlices >= 1, epR_InvalidArgument, "Invalid args!");

  size_t numVerts = 2 + numSegments*(numSlices+1);
  size_t numIndices = numSegments*2*(numSlices+1)*3;

  spVB->Allocate<Float3>(numVerts);
  spIB->Allocate<uint16_t>(numIndices);

  Slice<Float3> vb = spVB->Map<Float3>();
  epscope(exit) { spVB->Unmap(); };
  Slice<uint16_t> ib = spIB->Map<uint16_t>();
  epscope(exit) { spIB->Unmap(); };

  size_t v = 0;
  size_t i = 0;

  // top and bottom center verts
  vb[v++] = Float3{ 0.0, 0.0, 1.0 };
  vb[numVerts - 1] = Float3{ 0.0, 0.0, 0.0 };

  for (int s = 0; s < numSlices+1; ++s)
  {
    float z = float(1.0 - s/double(numSlices));

    for (int j = 0; j < numSegments+1; ++j)
    {
      // insert triangles
      if (j > 0)
      {
        size_t wrap = j == numSegments ? numSegments : 0;
        if (s == 0)
        {
          // top cap
          ib[i++] = (uint16_t)(0);
          ib[i++] = (uint16_t)(v-1);
          ib[i++] = (uint16_t)(v-wrap);
        }
        else
        {
          // central surface
          ib[i++] = (uint16_t)(v-wrap-numSegments);
          ib[i++] = (uint16_t)(v-numSegments-1);
          ib[i++] = (uint16_t)(v-wrap);
          ib[i++] = (uint16_t)(v-numSegments-1);
          ib[i++] = (uint16_t)(v-1);
          ib[i++] = (uint16_t)(v-wrap);

          if (s == numSlices)
          {
            ib[i++] = (uint16_t)(numVerts-1);
            ib[i++] = (uint16_t)(v-wrap);
            ib[i++] = (uint16_t)(v-1);
          }
        }
      }

      // insert the vertex
      if (j < numSegments)
      {
        // add vertex
        double a = EP_2PI*double(j) / double(numSegments);
        vb[v++] = Float3{ (float)(Sin(a)*0.5), (float)(Cos(a)*0.5), (float)z };
      }
    }
  }

  EPASSERT(++v == numVerts, "Incorrect number of vertices!");
  EPASSERT(i == numIndices, "Incorrect number of indices!");

  if (transformVertex)
  {
    for (Float3 &vert : vb)
      vert = transformVertex(vert);
  }
}

void PrimitiveGenerator::GenerateCone(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex)
{
  EPASSERT_THROW(numSegments >= 3 && numSlices >= 1, epR_InvalidArgument, "Invalid args!");

  size_t numVerts = 2 + numSegments*numSlices;
  size_t numIndices = numSegments*numSlices*2*3;

  spVB->Allocate<Float3>(numVerts);
  spIB->Allocate<uint16_t>(numIndices);

  Slice<Float3> vb = spVB->Map<Float3>();
  epscope(exit) { spVB->Unmap(); };
  Slice<uint16_t> ib = spIB->Map<uint16_t>();
  epscope(exit) { spIB->Unmap(); };

  size_t v = 0;
  size_t i = 0;

  // top and bottom center verts
  vb[v++] = Float3{ 0.0, 0.0, 1.0 };
  vb[numVerts - 1] = Float3{ 0.0, 0.0, 0.0 };

  for (int s = 0; s < numSlices; ++s)
  {
    double ratio = (s + 1) / double(numSlices);
    float z = float(1.0 - ratio);

    for (int j = 0; j < numSegments + 1; ++j)
    {
      // insert triangles
      if (j > 0)
      {
        size_t wrap = j == numSegments ? numSegments : 0;
        if (s == 0)
        {
          // top cap
          ib[i++] = (uint16_t)(0);
          ib[i++] = (uint16_t)(v - 1);
          ib[i++] = (uint16_t)(v - wrap);
        }
        else
        {
          // central surface8
          ib[i++] = (uint16_t)(v - wrap - numSegments);
          ib[i++] = (uint16_t)(v - numSegments - 1);
          ib[i++] = (uint16_t)(v - wrap);
          ib[i++] = (uint16_t)(v - numSegments - 1);
          ib[i++] = (uint16_t)(v - 1);
          ib[i++] = (uint16_t)(v - wrap);
        }
        if (s == numSlices-1)
        {
          ib[i++] = (uint16_t)(numVerts - 1);
          ib[i++] = (uint16_t)(v - wrap);
          ib[i++] = (uint16_t)(v - 1);
        }
      }

      // insert the vertex
      if (j < numSegments)
      {
        // add vertex
        double a = EP_2PI*double(j) / double(numSegments);
        vb[v++] = Float3{ (float)(Sin(a)*0.5*ratio), (float)(Cos(a)*0.5*ratio), (float)z };
      }
    }
  }

  EPASSERT(++v == numVerts, "Incorrect number of vertices!");
  EPASSERT(i == numIndices, "Incorrect number of indices!");

  if (transformVertex)
  {
    for (Float3 &vert : vb)
      vert = transformVertex(vert);
  }
}

} // namespace ep
