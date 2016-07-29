#pragma once
#if !defined(_EP_PRIMITIVEGENERATOR_H)
#define _EP_PRIMITIVEGENERATOR_H

#include "ep/cpp/internal/i/iprimitivegenerator.h"

namespace ep {

SHARED_CLASS(PrimitiveGenerator);
SHARED_CLASS(ArrayBuffer);

class PrimitiveGenerator : public Component
{
  EP_DECLARE_COMPONENT_WITH_STATIC_IMPL(ep, PrimitiveGenerator, IPrimitiveGenerator, IPrimitiveGeneratorStatic, Component, EPKERNEL_PLUGINVERSION, "Generate vertex buffers for primitive shapes", ComponentInfoFlags::Abstract)
public:

  static void generateQuad(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex = nullptr) { getStaticImpl()->GenerateQuad(spVB, spIB, transformVertex); }
  static void generateCube(ArrayBufferRef spVB, ArrayBufferRef spIB, Delegate<Float3(Float3)> transformVertex = nullptr) { getStaticImpl()->GenerateCube(spVB, spIB, transformVertex); }
  static void generateSphere(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 6, Delegate<Float3(Float3)> transformVertex = nullptr) { getStaticImpl()->GenerateSphere(spVB, spIB, numSegments, numSlices, transformVertex); }
  //! Generates a line circle model.
  //! \param spVB The vertex buffer to fill.
  //! \param spIB The index buffer to fill.
  //! \param numSegments The number of line segments to draw this model with.
  //! \param arcLength The length of the arc to draw. EP_2PI will produce a full circle.
  //! \param transformVertex Optional function to transform the generated points.
  //! \return None.
  //! \remarks Note: Throws if numSegments is less than 3.
  static void generateCircle(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, double arcLength = EP_2PI, Delegate<Float3(Float3)> transformVertex = nullptr) { getStaticImpl()->GenerateCircle(spVB, spIB, numSegments, arcLength, transformVertex); }
  //! Generates a disc model.
  //! \param spVB The vertex buffer to fill.
  //! \param spIB The index buffer to fill.
  //! \param numSegments The number of line segments to draw this model with.
  //! \param arcLength The length of the arc to draw. EP_2PI will produce a full disc.
  //! \param innerRadius The inner radius to produce the disc from; 0 will make a solid disc.
  //! \param transformVertex Optional function to transform the generated points.
  //! \return None.
  //! \remarks Note: Throws if numSegments is less than 3.
  static void generateDisc(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, double arcLength = EP_2PI, double innerRadius = 0, Delegate<Float3(Float3)> transformVertex = nullptr) { getStaticImpl()->GenerateDisc(spVB, spIB, numSegments, arcLength, innerRadius, transformVertex); }
  static void generateCylinder(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 1, Delegate<Float3(Float3)> transformVertex = nullptr) { getStaticImpl()->GenerateCylinder(spVB, spIB, numSegments, numSlices, transformVertex); }
  static void generateCone(ArrayBufferRef spVB, ArrayBufferRef spIB, int numSegments = 12, int numSlices = 1, Delegate<Float3(Float3)> transformVertex = nullptr) { getStaticImpl()->GenerateCone(spVB, spIB, numSegments, numSlices, transformVertex); }

protected:
  PrimitiveGenerator(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }
  ~PrimitiveGenerator()
  {}

  Array<const StaticFuncInfo> getStaticFuncs() const;
};

}

#endif // _EP_PRIMITIVEGENERATOR_H
