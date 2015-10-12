#pragma once
#ifndef EPNODE_H
#define EPNODE_H

#include "udMath.h"

#include "../resources/resource.h"
#include "hal/input.h"

namespace ep
{

SHARED_CLASS(RenderScene);

PROTOTYPE_COMPONENT(Node);

class Node : public Resource
{
public:
  EP_COMPONENT(Node);

  virtual void SetMatrix(const udDouble4x4 &mat) { matrix = mat; }
  const udDouble4x4& GetMatrix() const { return matrix; }

  virtual void SetPosition(const udDouble3 &pos) { matrix.axis.t = udDouble4::create(pos, matrix.axis.t.w); }
  const udDouble3& GetPosition() const { return matrix.axis.t.toVector3(); }

  NodeRef Parent() const { return NodeRef(pParent); }
  const epSlice<NodeRef> Children() const { return children; }

  void AddChild(NodeRef c);
  void RemoveChild(NodeRef c);

  void Detach();

  void CalculateWorldMatrix(udDouble4x4 *pMatrix) const;

protected:
  friend class Scene;

  Node(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  virtual bool InputEvent(const epInputEvent &ev);
  virtual bool Update(double timeStep);
  virtual epResult Render(RenderSceneRef &spScene, const udDouble4x4 &mat);

  Node *pParent = nullptr;
  epArray<NodeRef, 3> children;

  udDouble4x4 matrix = udDouble4x4::identity();

  // TODO: enable/visible/etc flags
};

} // namespace ep

#endif // EPNODE_H
