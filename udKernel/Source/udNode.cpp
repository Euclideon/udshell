#include "udNode.h"


static const udPropertyDesc props[] =
{
  {
    "matrix", // id
    "Matrix", // displayName
    "Local matrix", // description
    udGetter(&udNode::GetMatrix), // getter
    udSetter(&udNode::SetMatrix), // setter
    udTypeDesc(udPropertyType::Float, 16) // type
  },
  {
    "position", // id
    "Position", // displayName
    "Local position", // description
    udGetter(&udNode::GetPosition), // getter
    udSetter(&udNode::SetPosition), // setter
    udTypeDesc(udPropertyType::Float, 3) // type
  },
  {
    "parent", // id
    "Parent", // displayName
    "Parent node", // description
    udGetter(&udNode::GetPosition), // getter
    udSetter(&udNode::SetPosition), // setter
    udTypeDesc(udPropertyType::Component) // type
  },
  {
    "children", // id
    "Children", // displayName
    "Child nodes", // description
    udGetter(&udNode::GetPosition), // getter
    udSetter(&udNode::SetPosition), // setter
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
  [](){ return udR_Success; },             // pInitRender
  udNode::Create, // pCreateInstance

  udSlice<const udPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};


void udNode::CalculateWorldMatrix(udDouble4x4 *pMatrix) const
{
  // TODO: multiply local matrix by parent matrices
}
