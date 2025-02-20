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
};

class PrimitiveGeneratorImplStatic : public BaseStaticImpl<IPrimitiveGeneratorStatic>
{
public:
  void GenerateQuad(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex) override final;
  void GenerateCube(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex) override final;
  void GenerateSphere(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex) override final;
  void GenerateCircle(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, double arcLength, Delegate<Float3(Float3)> transformVertex) override final;
  void GenerateDisc(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, double arcLength, double innerRadius, Delegate<Float3(Float3)> transformVertex) override final;
  void GenerateCylinder(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex) override final;
  void GenerateCone(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments, int numSlices, Delegate<Float3(Float3)> transformVertex) override final;
};

}

#endif // _EP_PRIMITIVEGENERATORIMPL_H
