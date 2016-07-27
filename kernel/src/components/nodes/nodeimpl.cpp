#include "components/nodes/nodeimpl.h"

namespace ep {

Array<const PropertyInfo> Node::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("matrix", getMatrix, setMatrix, "Local matrix", nullptr, 0),
    EP_MAKE_PROPERTY("position", getPosition, setPosition, "Local position", nullptr, 0),
    EP_MAKE_PROPERTY_RO("parent", parent, "Parent node", nullptr, 0),
    EP_MAKE_PROPERTY_RO("children", children, "Child nodes", nullptr, 0),
  };
}
Array<const MethodInfo> Node::getMethods() const
{
  return{
    EP_MAKE_METHOD(addChild, "Add a child to the Node"),
    EP_MAKE_METHOD(removeChild, "Remove a child from the Node"),
    EP_MAKE_METHOD(detach, "Detach the Node from its parent"),
    EP_MAKE_METHOD_EXPLICIT("CalculateWorldMatrix", calculateWorldMatrixMethod, "Calculate the World Matrix of the Node"),
    EP_MAKE_METHOD(save, "Save the state of the Node"),
  };
}

void NodeImpl::DoRender(RenderScene &spScene, const Double4x4 &mat)
{
  pInstance->render(spScene, mat);

  for (NodeRef &n : children)
    n->doRender(spScene, mat * n->getMatrix());
}

bool NodeImpl::InputEvent(const ep::InputEvent &ev)
{
  for (auto &c : children)
  {
    if (c->inputEvent(ev))
      return true;
  }
  return false;
}

bool NodeImpl::Update(double timeStep)
{
  for (auto &c : children)
  {
    if (c->update(timeStep))
      return true;
  }
  return false;
}

void NodeImpl::AddChild(NodeRef c)
{
  NodeImpl *pC = c->GetImpl<NodeImpl>();
  EPASSERT_THROW(pC->pParent == nullptr, Result::InvalidArgument, "Node is already present in a scene");

  c->changed.Subscribe(this, &NodeImpl::OnChildChanged);

  children.concat(c);
  pC->pParent = pInstance;
}

void NodeImpl::RemoveChild(NodeRef c)
{
  NodeImpl *pC = c->GetImpl<NodeImpl>();
  EPASSERT_THROW(pC->pParent == pInstance, Result::InvalidArgument, "Node is not a child");

  children.removeFirst(c);
  pC->pParent = nullptr;

  c->changed.Unsubscribe(this, &NodeImpl::OnChildChanged);
}

void NodeImpl::Detach()
{
  EPASSERT_THROW(pParent != nullptr, Result::Failure, "Node is not in scene");
  pParent->removeChild(NodeRef(pInstance));
}

void NodeImpl::CalculateWorldMatrix(Double4x4 *pMatrix) const
{
  if (pMatrix)
  {
    NodeRef spParent = Parent();
    if (spParent)
    {
      Double4x4 parentMatrix;
      spParent->calculateWorldMatrix(&parentMatrix);

      *pMatrix = Mul(parentMatrix, matrix);
    }
    else
      *pMatrix = matrix;
  }
}

} // namespace ep
