#pragma once
#ifndef UDSCENEGRAPH_H
#define UDSCENEGRAPH_H

#include "udComponent.h"
#include "udInput.h"
#include "udMath.h"


class udRenderable
{
  // TODO: has stuff
  // * textures
  // * vertex data
  // * shader
  // * constants
  // * render states
};

class udNode : public udComponent
{
public:
  virtual udResult InputEvent(const udInputEvent &ev) { return udR_Success; }
  virtual udResult Update(double timeStep) { return udR_Success; }
  virtual udResult Render(const udDouble4x4 &mat) { return udR_Success; }

  void CalculateWorldMatrix(udDouble4x4 *pMatrix) const;

  static const udComponentDesc descriptor;

protected:
  udNode(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
    : udComponent(pType, pKernel, uid, initParams) {}
  virtual ~udNode() {}

  udDouble4x4 matrix = udDouble4x4::identity();

  udNode *pParent = nullptr;
  udRCSlice<udNode> children;

  // TODO: enable/visible/etc flags

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udNode, pType, pKernel, uid, initParams);
  }
};

class udRenderScene
{
public:

protected:
  int refCount;
};

#endif
