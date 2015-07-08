#include "udSceneGraph.h"


static const udPropertyDesc props[] =
{
  {
    "matrix", // id
    "Matrix", // displayName
    "Local matrix", // description
    udPropertyType::Float, // type
    16, // arrayLength
    udPF_NoRead, // flags
    udPropertyDisplayType::Default, // displayType
    nullptr,
    nullptr
  }
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

  udSlice<const udPropertyDesc>(props, ARRAY_LENGTH(props)) // propeties
};


void udNode::CalculateWorldMatrix(udDouble4x4 *pMatrix) const
{
  // TODO: multiply local matrix by parent matrices
}
