#pragma once
#ifndef EPNODEIMPL_H
#define EPNODEIMPL_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/inode.h"

#include "ep/cpp/math.h"
#include "ep/cpp/component/resource/resource.h"
#include "hal/input.h"

namespace ep {

SHARED_CLASS(RenderScene);
SHARED_CLASS(Node);

class NodeImpl : public BaseImpl<Node, INode>
{
public:
  NodeImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  virtual void SetMatrix(const Double4x4 &mat) override final { matrix = mat; }
  const Double4x4& GetMatrix() const override final { return matrix; }

  virtual void SetPosition(const Double3 &pos) override final { matrix.axis.t = Double4::create(pos, matrix.axis.t.w); }
  const Double3& GetPosition() const override final { return matrix.axis.t.toVector3(); }

  NodeRef Parent() const override final { return NodeRef(pParent); }
  const Slice<NodeRef> Children() const override final { return children; }

  void AddChild(NodeRef c) override final;
  void RemoveChild(NodeRef c) override final;

  void Detach() override final;

  void CalculateWorldMatrix(Double4x4 *pMatrix) const override final;

  Variant Save() const override final { return pInstance->Super::save(); }

protected:
  friend class Scene;
  friend class SceneNodeImpl;

  bool InputEvent(const ep::InputEvent &ev) override final;
  bool Update(double timeStep) override final;
  void Render(RenderScene &spScene, const Double4x4 &mat) override final { }
  void DoRender(RenderScene & spScene, const Double4x4 & mat) override final;

  void OnChildChanged() const { pInstance->changed.signal(); }

  Node *pParent = nullptr;
  Array<NodeRef, 3> children;

  Double4x4 matrix = Double4x4::identity();

  // TODO: enable/visible/etc flags
};

} // namespace ep

#endif // EPNODEIMPL_H
