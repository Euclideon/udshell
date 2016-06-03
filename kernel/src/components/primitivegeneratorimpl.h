#pragma once
#if !defined(_EP_PRIMITIVEGENERATORIMPL_H)
#define _EP_PRIMITIVEGENERATORIMPL_H

#include "ep/cpp/component/primitivegenerator.h"

namespace ep {

SHARED_CLASS(PrimitiveGenerator);
SHARED_CLASS(ArrayBuffer);

class PrimitiveGeneratorImpl : public BaseImpl<PrimitiveGenerator, IPrimitiveGenerator>
{
public:

  PrimitiveGeneratorImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  void GenerateQuad(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex = nullptr) override final;
  void GenerateCube(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex = nullptr) override final;
  void GenerateSphere(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 6, Delegate<Float3(Float3)> transformVertex = nullptr) override final;
  void GenerateCylinder(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 1, Delegate<Float3(Float3)> transformVertex = nullptr) override final;
  void GenerateCone(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 1, Delegate<Float3(Float3)> transformVertex = nullptr) override final;
};

}

#endif // _EP_PRIMITIVEGENERATORIMPL_H
