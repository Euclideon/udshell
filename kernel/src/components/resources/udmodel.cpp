#include "udmodel.h"

namespace ep
{

ComponentDesc UDModel::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "udmodel", // id
  "UD Model", // displayName
  "UD model resource", // description
};

UDModel::~UDModel()
{
  if (pOctree)
    pOctree->pDestroy(pOctree);
}

} // namespace ep
