#pragma once
#ifndef UDNODE_H
#define UDNODE_H

#include "udMath.h"

#include "components/component.h"
#include "hal/input.h"

namespace ud
{

SHARED_CLASS(RenderScene);

PROTOTYPE_COMPONENT(Node);

class Node : public Component
{
public:
  UD_COMPONENT(Node);

  virtual udResult InputEvent(const udInputEvent &ev) { return udR_Success; }
  virtual udResult Update(double timeStep) { return udR_Success; }
  virtual udResult Render(RenderSceneRef &spScene, const udDouble4x4 &mat);

  virtual void SetMatrix(const udDouble4x4 &mat) { matrix = mat; }
  const udDouble4x4& GetMatrix() const { return matrix; }

  virtual void SetPosition(const udDouble3 &pos) { matrix.axis.t = udDouble4::create(pos, matrix.axis.t.w); }
  const udDouble3& GetPosition() const { return matrix.axis.t.toVector3(); }

  NodeRef Parent() const { return NodeRef(pParent); }
  const udSlice<NodeRef> Children() const { return children; }

  void AddChild(NodeRef c);
  void RemoveChild(NodeRef c);

  void Detach();

  void CalculateWorldMatrix(udDouble4x4 *pMatrix) const;

protected:
  Node(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Component(pType, pKernel, uid, initParams) {}
  virtual ~Node() {}

  Node *pParent = nullptr;
  udFixedSlice<NodeRef, 3> children;

  udDouble4x4 matrix = udDouble4x4::identity();

  // TODO: enable/visible/etc flags
};

} // namespace ud
#endif // UDNODE_H
