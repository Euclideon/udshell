#include "components/nodes/nodeimpl.h"

namespace ep {

Array<const PropertyInfo> Node::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(Matrix, "Local matrix", nullptr, 0),
    EP_MAKE_PROPERTY(Position, "Local position", nullptr, 0),
    EP_MAKE_PROPERTY_EXPLICIT("Parent", "Parent node", EP_MAKE_GETTER(Parent), nullptr, nullptr, 0),
    EP_MAKE_PROPERTY_EXPLICIT("Children", "Child nodes", EP_MAKE_GETTER(Children), nullptr, nullptr, 0),
  };
}
Array<const MethodInfo> Node::GetMethods() const
{
  return{
    EP_MAKE_METHOD(AddChild, "Add a child to the Node"),
    EP_MAKE_METHOD(RemoveChild, "Remove a child from the Node"),
    EP_MAKE_METHOD(Detach, "Detach the Node from its parent"),
    EP_MAKE_METHOD_EXPLICIT("CalculateWorldMatrix", CalculateWorldMatrixMethod, "Calculate the World Matrix of the Node"),
    EP_MAKE_METHOD(Save, "Save the state of the Node"),
  };
}

void NodeImpl::Render(RenderScene &spScene, const Double4x4 &mat)
{
  for (NodeRef &n : children)
    n->Render(spScene, mat * n->GetMatrix());
}

bool NodeImpl::InputEvent(const epInputEvent &ev)
{
  for (auto &c : children)
  {
    if (c->InputEvent(ev))
      return true;
  }
  return false;
}

bool NodeImpl::Update(double timeStep)
{
  for (auto &c : children)
  {
    if (c->Update(timeStep))
      return true;
  }
  return false;
}

void NodeImpl::AddChild(NodeRef c)
{
  children.concat(c);
}

void NodeImpl::RemoveChild(NodeRef c)
{
  children.removeFirst(c);
  ((NodeImpl *)c->pImpl.ptr())->pParent = nullptr; // TODO This is ugly, we need a better way to access the Impl of another component instance
}

void NodeImpl::Detach()
{
  if (pParent)
    pParent->RemoveChild(NodeRef(pInstance));
}

void NodeImpl::CalculateWorldMatrix(Double4x4 *pMatrix) const
{
  if (pMatrix)
  {
    NodeRef spParent = Parent();
    if (spParent)
    {
      Double4x4 parentMatrix;
      spParent->CalculateWorldMatrix(&parentMatrix);

      *pMatrix = Mul(parentMatrix, matrix);
    }
    else
      *pMatrix = matrix;
  }
}

} // namespace ep
