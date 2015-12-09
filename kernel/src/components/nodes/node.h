#pragma once
#ifndef EPNODE_H
#define EPNODE_H

#include "ep/cpp/math.h"

#include "../resources/resource.h"
#include "hal/input.h"

namespace ep
{

SHARED_CLASS(RenderScene);

SHARED_CLASS(Node);

class Node : public Resource
{
  EP_DECLARE_COMPONENT(Node, Resource, EPKERNEL_PLUGINVERSION, "Node desc...")
public:

  virtual void SetMatrix(const Double4x4 &mat) { matrix = mat; }
  const Double4x4& GetMatrix() const { return matrix; }

  virtual void SetPosition(const Double3 &pos) { matrix.axis.t = Double4::create(pos, matrix.axis.t.w); }
  const Double3& GetPosition() const { return matrix.axis.t.toVector3(); }

  NodeRef Parent() const { return NodeRef(pParent); }
  const Slice<NodeRef> Children() const { return children; }

  void AddChild(NodeRef c);
  void RemoveChild(NodeRef c);

  void Detach();

  void CalculateWorldMatrix(Double4x4 *pMatrix) const;

protected:
  friend class Scene;

  Node(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams) {}

  virtual bool InputEvent(const epInputEvent &ev);
  virtual bool Update(double timeStep);
  virtual epResult Render(RenderSceneRef &spScene, const Double4x4 &mat);

  Node *pParent = nullptr;
  Array<NodeRef, 3> children;

  Double4x4 matrix = Double4x4::identity();

  // TODO: enable/visible/etc flags

  static Array<const PropertyInfo> GetProperties();
};

} // namespace ep

#endif // EPNODE_H
