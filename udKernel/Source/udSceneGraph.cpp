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
    udPropertyDisplayType::Default // displayType
  }
};
const udComponentDesc udNode::descriptor =
{
  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  udComponentType::Node, // type

  "node",      // id
  "component", // parentId
  "udNode",    // displayName
  "Is a scene node", // description

  [](){ return udR_Success; },             // pInit
  [](){ return udR_Success; },             // pInitRender
  udNode::Create, // pCreateInstance

  props,                           // pProperties
  sizeof(props) / sizeof(props[0]) // numProperties
};


void udNode::CalculateWorldMatrix(udDouble4x4 *pMatrix) const
{
  // TODO: multiply local matrix by parent matrices
}
