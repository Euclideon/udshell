#include "node.h"

namespace ep
{

static CPropertyDesc props[] =
{
  {
    {
      "matrix", // id
      "Matrix", // displayName
      "Local matrix", // description
    },
    &Node::GetMatrix, // getter
    &Node::SetMatrix, // setter
  },
  {
    {
      "position", // id
      "Position", // displayName
      "Local position", // description
    },
    &Node::GetPosition, // getter
    &Node::SetPosition, // setter
  },
  {
    {
      "parent", // id
      "Parent", // displayName
      "Parent node", // description
    },
    &Node::GetPosition, // getter
    &Node::SetPosition, // setter
  },
  {
    {
      "children", // id
      "Children", // displayName
      "Child nodes", // description
    },
    &Node::GetPosition, // getter
    &Node::SetPosition, // setter
  },
};
ComponentDesc Node::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "node",      // id
  "Node",    // displayName
  "Is a scene node", // description

  epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};

udResult Node::Render(RenderSceneRef &spScene, const udDouble4x4 &mat)
{
  for (NodeRef &n : children)
    n->Render(spScene, mat * n->matrix);
  return udR_Success;
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

void Node::CalculateWorldMatrix(udDouble4x4 *pMatrix) const
{
  if (pMatrix)
  {
    NodeRef spParent = Parent();
    if (spParent)
    {
      udDouble4x4 parentMatrix;
      spParent->CalculateWorldMatrix(&parentMatrix);

      *pMatrix = udMul(parentMatrix, matrix);
    }
    else
      *pMatrix = matrix;
  }
}

} // namespace ep
