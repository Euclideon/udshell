#include "ep/cpp/component/resource/arraybuffer.h"
#include "components/primitivegeneratorimpl.h"

namespace ep {

Array<const StaticFuncInfo> PrimitiveGenerator::getStaticFuncs() const
{
  return{
    EP_MAKE_STATICFUNC(generateCube, "Generate a cube primitive"),
    EP_MAKE_STATICFUNC(generateSphere, "Generate a sphere primitive"),
    EP_MAKE_STATICFUNC(generateCircle, "Generate a line circle primitive"),
    EP_MAKE_STATICFUNC(generateCylinder, "Generate a cylinder primitive"),
    EP_MAKE_STATICFUNC(generateCone, "Generate a cone primitive"),
  };
}

void PrimitiveGeneratorImplStatic::GenerateQuad(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex)
{
  static const Float3 rawVB[] = {
    Float3{ -1.0f, -1.0f, 0.0f },
    Float3{  1.0f, -1.0f, 0.0f },
    Float3{ -1.0f,  1.0f, 0.0f },
    Float3{  1.0f,  1.0f, 0.0f },
  };

  spVB->allocateFromData(Slice<const Float3>(rawVB));

  Slice<Float3> vb = spVB->map<Float3>();
  epscope(exit) { spVB->unmap(); };

  if (transformVertex)
  {
    for (Float3 &v : vb)
      v = transformVertex(v);
  }

  static const uint16_t rawIB[] = { 0, 1, 2, 3, 2, 1 };
  spIB->allocateFromData(Slice<const uint16_t>(rawIB));
}

void PrimitiveGeneratorImplStatic::GenerateCube(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex)
{
  // vertex buffer
  spVB->allocate<Float3>(8);
  Slice<Float3> vb = spVB->map<Float3>();
  epscope(exit) { spVB->unmap(); };

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
  spIB->allocateFromData<uint16_t>(Slice<const uint16_t>
  {
    0,1,2, 1,3,2, // top
    2,3,6, 3,7,6, // front
    3,1,7, 1,5,7, // right
    1,0,5, 0,4,5, // back
    0,2,4, 2,6,4, // left
    7,6,4, 7,4,5  // bottom
  });
}

void PrimitiveGeneratorImplStatic::GenerateSphere(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex)
{
  EPASSERT_THROW(numSegments >= 3 && numSlices >= 2, Result::InvalidArgument, "Invalid args!");

  size_t numVerts = 2 + numSegments*(numSlices-1);
  size_t numIndices = numSegments*(numSlices-1)*2*3;

  spVB->allocate<Float3>(numVerts);
  spIB->allocate<uint16_t>(numIndices);

  Slice<Float3> vb = spVB->map<Float3>();
  epscope(exit) { spVB->unmap(); };
  Slice<uint16_t> ib = spIB->map<uint16_t>();
  epscope(exit) { spIB->unmap(); };

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

void PrimitiveGeneratorImplStatic::GenerateCircle(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, double arcLength, Delegate<Float3(Float3)> transformVertex)
{
  EPASSERT_THROW(numSegments >= 3, Result::InvalidArgument, "Circle must have at least 3 segments!");

  const double radius = .5;
  bool completeRotation = Abs(arcLength - EP_2PI) < 0.001;

  size_t numVerts = completeRotation ? numSegments : numSegments + 1; // if not complete rotation add a extra vertex.
  size_t numIndices = numSegments * 2;

  spVB->allocate<Float3>(numVerts);
  spIB->allocate<uint16_t>(numIndices);

  Slice<Float3> vb = spVB->map<Float3>();
  epscope(exit) { spVB->unmap(); };
  Slice<uint16_t> ib = spIB->map<uint16_t>();
  epscope(exit) { spIB->unmap(); };

  size_t v = 0;
  for (size_t j = 0; j < numVerts; ++j)
  {
    double a = arcLength*double(j) / numSegments;
    vb[v++] = Float3{ (float)(Sin(a)*radius), (float)(Cos(a)*radius), 0.0f };
  }

  size_t i = 0;
  for (uint16_t j = 0; j < numSegments; j++)
  {
    ib[i++] = j;
    ib[i++] = (j + 1) % (numVerts);
  }

  EPASSERT(v == numVerts, "Incorrect number of vertices!");
  EPASSERT(i == numIndices, "Incorrect number of indices!");

  if (transformVertex)
  {
    for (Float3 &vert : vb)
      vert = transformVertex(vert);
  }
}

void PrimitiveGeneratorImplStatic::GenerateDisc(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, double arcLength, double innerRadius, Delegate<Float3(Float3)> transformVertex)
{
  EPASSERT_THROW(numSegments >= 3, Result::InvalidArgument, "Disc must have at least 3 segments!");

  bool isDisc = innerRadius > 0.001; // is disc or circle
  bool completeRotation = Abs(arcLength - EP_2PI) < 0.001;

  size_t vertN = completeRotation ? numSegments : numSegments + 1;
  size_t numVerts = isDisc ? 2 * vertN : vertN + 1;
  size_t numIndices = isDisc ? numSegments * 3 * 2 : numSegments * 3;

  spVB->allocate<Float3>(numVerts);
  spIB->allocate<uint16_t>(numIndices);

  Slice<Float3> vb = spVB->map<Float3>();
  epscope(exit) { spVB->unmap(); };
  Slice<uint16_t> ib = spIB->map<uint16_t>();
  epscope(exit) { spIB->unmap(); };

  size_t v = 0, i = 0;

  // top and bottom center verts
  if (!isDisc)
    vb[v++] = Float3{ 0.0, 0.0, 0.0 };

  for (size_t j = 0; j < vertN; ++j)
  {
    double a = arcLength*double(j) / double(numSegments);
    vb[v++] = Float3{ (float)(Sin(a)*0.5), (float)(Cos(a)*0.5), 0.0f };
  }

  if (isDisc)
  {
    for (size_t j = 0; j < vertN; ++j)
    {
      double a = arcLength*double(j) / double(numSegments);
      vb[v++] = Float3{ (float)(Sin(a)*innerRadius), (float)(Cos(a)*innerRadius), 0.0f };
    }

    size_t loopCount = numSegments;

    for (size_t j = 0; j < loopCount; j++)
    {
      uint16_t it = (uint16_t)j;
      ib[i++] = it;
      ib[i++] = (it + 1) % vertN;
      ib[i++] = (uint16_t)vertN + it;
    }

    for (size_t j = 0; j < loopCount; j++)
    {
      uint16_t it = (uint16_t)j;
      ib[i++] = it + (uint16_t)vertN;
      ib[i++] = (it + 1) % vertN + (uint16_t)vertN;
      ib[i++] = (it + 1) % vertN;
    }
  }
  else
  {
    size_t loopCount = numSegments + 1;

    for (size_t j = 1; j < loopCount; j++)
    {
      uint16_t it = (uint16_t)j;
      ib[i++] = 0;
      ib[i++] = it;

      if (j == loopCount - 1 && completeRotation) // if complete rotation, loop back
        ib[i++] = 1;
      else
        ib[i++] = (it % numVerts) + (1);
    }
  }

  EPASSERT(v == numVerts, "Incorrect number of vertices!");
  EPASSERT(i == numIndices, "Incorrect number of indices!");

  if (transformVertex)
  {
    for (Float3 &vert : vb)
      vert = transformVertex(vert);
  }
}

void PrimitiveGeneratorImplStatic::GenerateCylinder(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex)
{
  EPASSERT_THROW(numSegments >= 3 && numSlices >= 1, Result::InvalidArgument, "Invalid args!");

  size_t numVerts = 2 + numSegments*(numSlices+1);
  size_t numIndices = numSegments*2*(numSlices+1)*3;

  spVB->allocate<Float3>(numVerts);
  spIB->allocate<uint16_t>(numIndices);

  Slice<Float3> vb = spVB->map<Float3>();
  epscope(exit) { spVB->unmap(); };
  Slice<uint16_t> ib = spIB->map<uint16_t>();
  epscope(exit) { spIB->unmap(); };

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

void PrimitiveGeneratorImplStatic::GenerateCone(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex)
{
  EPASSERT_THROW(numSegments >= 3 && numSlices >= 1, Result::InvalidArgument, "Invalid args!");

  size_t numVerts = 2 + numSegments*numSlices;
  size_t numIndices = numSegments*numSlices*2*3;

  spVB->allocate<Float3>(numVerts);
  spIB->allocate<uint16_t>(numIndices);

  Slice<Float3> vb = spVB->map<Float3>();
  epscope(exit) { spVB->unmap(); };
  Slice<uint16_t> ib = spIB->map<uint16_t>();
  epscope(exit) { spIB->unmap(); };

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
