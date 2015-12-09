#include "node.h"

namespace ep
{

Array<const PropertyInfo> Node::GetProperties()
{
  return{
    EP_MAKE_PROPERTY(Matrix, "Local matrix", nullptr, 0),
    EP_MAKE_PROPERTY(Position, "Local position", nullptr, 0),
    EP_MAKE_PROPERTY_EXPLICIT("Parent", "Parent node", EP_MAKE_GETTER(Parent), nullptr, nullptr, 0),
    EP_MAKE_PROPERTY_EXPLICIT("Children", "Child nodes", EP_MAKE_GETTER(Children), nullptr, nullptr, 0),
  };
}

epResult Node::Render(RenderSceneRef &spScene, const Double4x4 &mat)
{
  for (NodeRef &n : children)
    n->Render(spScene, mat * n->matrix);
  return epR_Success;
}

bool Node::InputEvent(const epInputEvent &ev)
{
  for (auto &c : children)
  {
    if (c->InputEvent(ev))
      return true;
  }
  return false;
}

bool Node::Update(double timeStep)
{
  for (auto &c : children)
  {
    if (c->Update(timeStep))
      return true;
  }
  return false;
}

void Node::AddChild(NodeRef c)
{
  children.concat(c);
}

void Node::RemoveChild(NodeRef c)
{
  children.removeFirst(c);
  c->pParent = nullptr;
}

void Node::Detach()
{
  if (pParent)
    pParent->RemoveChild(NodeRef(this));
}

void Node::CalculateWorldMatrix(Double4x4 *pMatrix) const
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
