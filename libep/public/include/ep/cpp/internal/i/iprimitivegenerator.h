#pragma once
#if !defined(_EP_IPRIMITIVEGENERATOR_H)
#define _EP_IPRIMITIVEGENERATOR_H

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(PrimitiveGenerator);
SHARED_CLASS(ArrayBuffer);

class IPrimitiveGenerator
{
};

class IPrimitiveGeneratorStatic
{
public:
  virtual void GenerateQuad(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex) = 0;
  virtual void GenerateCube(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex) = 0;
  virtual void GenerateSphere(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex) = 0;
  virtual void GenerateCircle(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, double arcLength, Delegate<Float3(Float3)> transformVertex) = 0;
  virtual void GenerateDisc(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, double arcLength, double innerRadius, Delegate<Float3(Float3)> transformVertex) = 0;
  virtual void GenerateCylinder(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex) = 0;
  virtual void GenerateCone(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex) = 0;
};

}

#endif // _EP_IPRIMITIVEGENERATOR_H
