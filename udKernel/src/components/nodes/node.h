#pragma once
#ifndef UDNODE_H
#define UDNODE_H

#include "udMath.h"

#include "../resources/resource.h"
#include "hal/input.h"

namespace ud
{

SHARED_CLASS(RenderScene);

PROTOTYPE_COMPONENT(Node);

class Node : public Resource
{
public:
  UD_COMPONENT(Node);

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
  friend class Scene;

  Node(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  virtual bool InputEvent(const udInputEvent &ev);
  virtual bool Update(double timeStep);
  virtual udResult Render(RenderSceneRef &spScene, const udDouble4x4 &mat);

  Node *pParent = nullptr;
  udFixedSlice<NodeRef, 3> children;

  udDouble4x4 matrix = udDouble4x4::identity();

  // TODO: enable/visible/etc flags
};

} // namespace ud

#endif // UDNODE_H
