#include "node.h"

namespace ud
{

static PropertyDesc props[] =
{
  {
    {
      "matrix", // id
      "Matrix", // displayName
      "Local matrix", // description
      TypeDesc(PropertyType::Float, 16) // type
    },
    &Node::GetMatrix, // getter
    &Node::SetMatrix, // setter
  },
  {
    {
      "position", // id
      "Position", // displayName
      "Local position", // description
      TypeDesc(PropertyType::Float, 3) // type
    },
    &Node::GetPosition, // getter
    &Node::SetPosition, // setter
  },
  {
    {
      "parent", // id
      "Parent", // displayName
      "Parent node", // description
      TypeDesc(PropertyType::Component) // type
    },
    &Node::GetPosition, // getter
    &Node::SetPosition, // setter
  },
  {
    {
      "children", // id
      "Children", // displayName
      "Child nodes", // description
      TypeDesc(PropertyType::Component, ~0U) // type
    },
    &Node::GetPosition, // getter
    &Node::SetPosition, // setter
  },
};
ComponentDesc Node::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "node",      // id
  "Node",    // displayName
  "Is a scene node", // description

  udSlice<PropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};

udResult Node::Render(RenderSceneRef &spScene, const udDouble4x4 &mat)
{
  for (NodeRef &n : children)
    n->Render(spScene, mat * n->matrix);
  return udR_Success;
}

void Node::AddChild(NodeRef c)
{
  children.concat(c);
}

void Node::RemoveChild(NodeRef c)
{
  children.remove(c);
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

} // namespace ud
