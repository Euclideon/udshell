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
  virtual void setMatrix(const Double4x4 &mat) { pImpl->SetMatrix(mat); }
  virtual const Double4x4& getMatrix() const { return pImpl->GetMatrix(); }

  virtual void setPosition(const Double3 &pos) { pImpl->SetPosition(pos); }
  virtual const Double3& getPosition() const { return pImpl->GetPosition(); }

  virtual NodeRef parent() const { return pImpl->Parent(); }
  virtual const Slice<NodeRef> children() const { return pImpl->Children(); }

  virtual void addChild(NodeRef c) { pImpl->AddChild(c); }
  virtual void removeChild(NodeRef c) { pImpl->RemoveChild(c); }

  virtual void detach() { pImpl->Detach(); }

  void calculateWorldMatrix(Double4x4 *pMatrix) const { pImpl->CalculateWorldMatrix(pMatrix); }

  Variant save() const override { return pImpl->Save(); }

protected:
  Node(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  virtual bool inputEvent(const ep::InputEvent &ev) { return pImpl->InputEvent(ev); }
  virtual bool update(double timeStep) { return pImpl->Update(timeStep); }
  virtual void render(RenderScene &spScene, const Double4x4 &mat) { pImpl->Render(spScene, mat); }

  // TODO: enable/visible/etc flags

private:
  Double4x4 calculateWorldMatrixMethod() const
  {
    Double4x4 _matrix;
    calculateWorldMatrix(&_matrix);
    return _matrix;
  }

  void doRender(RenderScene &spScene, const Double4x4 &mat) { pImpl->DoRender(spScene, mat); }
  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

} // namespace ep

#endif // EPNODE_H
