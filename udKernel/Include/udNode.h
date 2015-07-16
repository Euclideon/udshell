#pragma once
#ifndef UDNODE_H
#define UDNODE_H

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

class udNode;
PROTOTYPE_COMPONENT(udNode);

class udNode : public udComponent
{
public:
  UD_COMPONENT(udNode);

  virtual udResult InputEvent(const udInputEvent &ev) { return udR_Success; }
  virtual udResult Update(double timeStep) { return udR_Success; }
  virtual udResult Render(const udDouble4x4 &mat) { return udR_Success; }

  virtual void SetMatrix(const udDouble4x4 &mat) { matrix = mat; }
  const udDouble4x4& GetMatrix() const { return matrix; }

  virtual void SetPosition(const udDouble3 &pos) { matrix.axis.t = udDouble4::create(pos, matrix.axis.t.w); }
  const udDouble3& GetPosition() const { return matrix.axis.t.toVector3(); }

  udNodeRef Parent() const { return parent; }
  const udSlice<udNodeRef> Children() const { return children; }

  void CalculateWorldMatrix(udDouble4x4 *pMatrix) const;

protected:
  udNode(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
    : udComponent(pType, pKernel, uid, initParams) {}
  virtual ~udNode() {}

  udDouble4x4 matrix = udDouble4x4::identity();

  udNodeRef parent;
  udRCSlice<udNodeRef> children;

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

#endif // UDNODE_H
