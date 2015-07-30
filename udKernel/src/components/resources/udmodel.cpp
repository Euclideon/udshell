#include "udmodel.h"

namespace ud
{

ComponentDesc UDModel::descriptor =
{
  &Resource::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "udmodel", // id
  "UD Model", // displayName
  "UD model resource", // description
};

void UDModel::Load(udString name, bool useStreamer)
{
  udOctree_Create(&pOctree, name.toStringz(), useStreamer, 0);
}

UDModel::~UDModel()
{
  if (pOctree)
    pOctree->pDestroy(pOctree);
}

} // namespace ud
