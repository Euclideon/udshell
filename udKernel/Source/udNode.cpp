#include "udNode.h"


static const udPropertyDesc props[] =
{
  {
    "matrix", // id
    "Matrix", // displayName
    "Local matrix", // description
    udPropertyType::Float, // type
    16, // arrayLength
    0, // flags
    udPropertyDisplayType::Default, // displayType
    udGetter(&udNode::GetMatrix),
    udSetter(&udNode::SetMatrix)
  },
  {
    "position", // id
    "Position", // displayName
    "Local position", // description
    udPropertyType::Float, // type
    3, // arrayLength
    0, // flags
    udPropertyDisplayType::Default, // displayType
    udGetter(&udNode::GetPosition),
    udSetter(&udNode::SetPosition)
  },
  {
    "parent", // id
    "Parent", // displayName
    "Parent node", // description
    udPropertyType::Component, // type
    0, // arrayLength
    0, // flags
    udPropertyDisplayType::Default, // displayType
    udGetter(&udNode::GetPosition),
    udSetter(&udNode::SetPosition)
  },
  {
    "children", // id
    "Children", // displayName
    "Child nodes", // description
    udPropertyType::Component, // type
    -1, // arrayLength
    0, // flags
    udPropertyDisplayType::Default, // displayType
    udGetter(&udNode::GetPosition),
    udSetter(&udNode::SetPosition)
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
