#pragma once
#if !defined(_EP_PRIMITIVEGENERATOR_H)
#define _EP_PRIMITIVEGENERATOR_H

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(PrimitiveGenerator);
SHARED_CLASS(ArrayBuffer);

class PrimitiveGenerator : public Component
{
  EP_DECLARE_COMPONENT(ep, PrimitiveGenerator, Component, EPKERNEL_PLUGINVERSION, "Generate vertex buffers for primitive shapes", 0)
public:

  void GenerateCube(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex = nullptr);
  void GenerateSphere(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 6, Delegate<Float3(Float3)> transformVertex = nullptr);
  void GenerateCylinder(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 1, Delegate<Float3(Float3)> transformVertex = nullptr);
  void GenerateCone(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 1, Delegate<Float3(Float3)> transformVertex = nullptr);

protected:
  PrimitiveGenerator(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {}
  ~PrimitiveGenerator()
  {}

  Array<const MethodInfo> GetMethods() const;
};

}

#endif // _EP_PRIMITIVEGENERATOR_H
