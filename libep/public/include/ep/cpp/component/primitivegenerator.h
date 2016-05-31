#pragma once
#if !defined(_EP_PRIMITIVEGENERATOR_H)
#define _EP_PRIMITIVEGENERATOR_H

#include "ep/cpp/internal/i/iprimitivegenerator.h"

namespace ep {

SHARED_CLASS(PrimitiveGenerator);
SHARED_CLASS(ArrayBuffer);

class PrimitiveGenerator : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, PrimitiveGenerator, IPrimitiveGenerator, Component, EPKERNEL_PLUGINVERSION, "Generate vertex buffers for primitive shapes", 0)
public:

  void GenerateQuad(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex = nullptr) { pImpl->GenerateQuad(spVB, spIB, transformVertex); }
  void GenerateCube(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex = nullptr) { pImpl->GenerateCube(spVB, spIB, transformVertex); }
  void GenerateSphere(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 6, Delegate<Float3(Float3)> transformVertex = nullptr) { pImpl->GenerateSphere(spVB, spIB, numSegments, numSlices, transformVertex); }
  void GenerateCylinder(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 1, Delegate<Float3(Float3)> transformVertex = nullptr) { pImpl->GenerateCylinder(spVB, spIB, numSegments, numSlices, transformVertex); }
  void GenerateCone(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 1, Delegate<Float3(Float3)> transformVertex = nullptr) { pImpl->GenerateCone(spVB, spIB, numSegments, numSlices, transformVertex); }

protected:
  PrimitiveGenerator(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }
  ~PrimitiveGenerator()
  {}

  Array<const MethodInfo> GetMethods() const;
};

}

#endif // _EP_PRIMITIVEGENERATOR_H
