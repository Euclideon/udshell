#include "components/nodes/nodeimpl.h"

namespace ep {

epResult NodeImpl::Render(RenderSceneRef &spScene, const Double4x4 &mat)
{
  for (NodeRef &n : children)
    n->Render(spScene, mat * n->GetMatrix());
  return epR_Success;
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
