#pragma once
#ifndef EPNODE_H
#define EPNODE_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/inode.h"

#include "ep/cpp/math.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/input.h"

namespace ep {

SHARED_CLASS(Node);
SHARED_CLASS(Scene);
SHARED_CLASS(RenderScene); // TODO Move RenderScene to epLib

class Node : public Resource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Node, INode, Resource, EPKERNEL_PLUGINVERSION, "Node desc...", 0)
public:
  friend class Scene;
  friend class SceneImpl;
  friend class SceneNodeImpl;

  // TODO: i don't think all these functions should be virtual; consider each one!
  virtual void SetMatrix(const Double4x4 &mat) { pImpl->SetMatrix(mat); }
  virtual const Double4x4& GetMatrix() const { return pImpl->GetMatrix(); }

  virtual void SetPosition(const Double3 &pos) { pImpl->SetPosition(pos); }
  virtual const Double3& GetPosition() const { return pImpl->GetPosition(); }

  virtual NodeRef Parent() const { return pImpl->Parent(); }
  virtual const Slice<NodeRef> Children() const { return pImpl->Children(); }

  virtual void AddChild(NodeRef c) { pImpl->AddChild(c); }
  virtual void RemoveChild(NodeRef c) { pImpl->RemoveChild(c); }

  virtual void Detach() { pImpl->Detach(); }

  void CalculateWorldMatrix(Double4x4 *pMatrix) const { pImpl->CalculateWorldMatrix(pMatrix); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  Node(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  virtual bool InputEvent(const ep::InputEvent &ev) { return pImpl->InputEvent(ev); }
  virtual bool Update(double timeStep) { return pImpl->Update(timeStep); }
  virtual void Render(RenderScene &spScene, const Double4x4 &mat) { pImpl->Render(spScene, mat); }

  // TODO: enable/visible/etc flags

private:
  Double4x4 CalculateWorldMatrixMethod() const
  {
    Double4x4 _matrix;
    CalculateWorldMatrix(&_matrix);
    return _matrix;
  }

  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

} // namespace ep

#endif // EPNODE_H
