#include "udNode.h"


static const udPropertyDesc props[] =
{
  {
    "matrix", // id
    "Matrix", // displayName
    "Local matrix", // description
    &udNode::GetMatrix, // getter
    &udNode::SetMatrix, // setter
    udTypeDesc(udPropertyType::Float, 16) // type
  },
  {
    "position", // id
    "Position", // displayName
    "Local position", // description
    &udNode::GetPosition, // getter
    &udNode::SetPosition, // setter
    udTypeDesc(udPropertyType::Float, 3) // type
  },
  {
    "parent", // id
    "Parent", // displayName
    "Parent node", // description
    &udNode::GetPosition, // getter
    &udNode::SetPosition, // setter
    udTypeDesc(udPropertyType::Component) // type
  },
  {
    "children", // id
    "Children", // displayName
    "Child nodes", // description
    &udNode::GetPosition, // getter
    &udNode::SetPosition, // setter
    udTypeDesc(udPropertyType::Component, ~0U) // type
  },
};
const udComponentDesc udNode::descriptor =
{
  &udComponent::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "node",      // id
  "udNode",    // displayName
  "Is a scene node", // description

  [](){ return udR_Success; },             // pInit
  udNode::Create, // pCreateInstance

  udSlice<const udPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};

udResult udNode::Render(udRenderSceneRef &spScene, const udDouble4x4 &mat)
{
  for (udNodeRef &n : children)
    n->Render(spScene, mat * n->matrix);
  return udR_Success;
}

void udNode::AddChild(udNodeRef c)
{
  children.concat(c);
}

void udNode::RemoveChild(udNodeRef c)
{
  children.remove(c);
  c->pParent = nullptr;
}

void udNode::Detach()
{
  if (pParent)
    pParent->RemoveChild(udNodeRef(this));
}

void udNode::CalculateWorldMatrix(udDouble4x4 *pMatrix) const
{
  if (pMatrix)
  {
    udNodeRef spParent = Parent();
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
