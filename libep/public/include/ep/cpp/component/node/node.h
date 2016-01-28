#pragma once
#ifndef EPNODE_H
#define EPNODE_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/inode.h"

#include "ep/cpp/math.h"
#include "ep/cpp/component/resource/resource.h"
#include "ep/c/input.h"

namespace ep
{

SHARED_CLASS(Node);
SHARED_CLASS(Scene);
SHARED_CLASS(RenderScene); // TODO Move RenderScene to epLib

class Node : public Resource, public INode
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Node, INode, Resource, EPKERNEL_PLUGINVERSION, "Node desc...")
public:
  friend class Scene;
  friend class SceneImpl;

  void SetMatrix(const Double4x4 &mat) override { pImpl->SetMatrix(mat); }
  const Double4x4& GetMatrix() const override { return pImpl->GetMatrix(); }

  void SetPosition(const Double3 &pos) override { pImpl->SetPosition(pos); }
  const Double3& GetPosition() const override { return pImpl->GetPosition(); }

  NodeRef Parent() const override { return pImpl->Parent(); }
  const Slice<NodeRef> Children() const override { return pImpl->Children(); }

  void AddChild(NodeRef c) override { pImpl->AddChild(c); }
  void RemoveChild(NodeRef c) override { pImpl->RemoveChild(c); }

  void Detach() override { pImpl->Detach(); }

  void CalculateWorldMatrix(Double4x4 *pMatrix) const override { pImpl->CalculateWorldMatrix(pMatrix); }

  Variant Save() const override { return pImpl->Save(); }
protected:
  Node(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  bool InputEvent(const epInputEvent &ev) override { return pImpl->InputEvent(ev); }
  bool Update(double timeStep) override { return pImpl->Update(timeStep); }
  void Render(RenderScene &spScene, const Double4x4 &mat) override { pImpl->Render(spScene, mat); }

  // TODO: enable/visible/etc flags

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(Matrix, "Local matrix", nullptr, 0),
      EP_MAKE_PROPERTY(Position, "Local position", nullptr, 0),
      EP_MAKE_PROPERTY_EXPLICIT("Parent", "Parent node", EP_MAKE_GETTER(Parent), nullptr, nullptr, 0),
      EP_MAKE_PROPERTY_EXPLICIT("Children", "Child nodes", EP_MAKE_GETTER(Children), nullptr, nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(AddChild, "Add a child to the Node"),
      EP_MAKE_METHOD(RemoveChild, "Remove a child from the Node"),
      EP_MAKE_METHOD(Detach, "Detach the Node from its parent"),
      EP_MAKE_METHOD_EXPLICIT("CalculateWorldMatrix", CalculateWorldMatrixMethod, "Calculate the World Matrix of the Node"),
      EP_MAKE_METHOD(Save, "Save the state of the Node"),
    };
  }

private:
  Double4x4 CalculateWorldMatrixMethod() const
  {
    Double4x4 _matrix;
    CalculateWorldMatrix(&_matrix);
    return _matrix;
  }
};

} // namespace ep

#endif // EPNODE_H
