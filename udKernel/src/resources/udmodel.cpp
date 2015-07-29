#include "udmodel.h"

namespace ud
{

UDModelRef UDModel::Create(udString name, bool useStreamer)
{
  udResult result;
  UDModelRef spModel(nullptr);

  UDModel *pSharedModel = udNew(UDModel);
  UD_ERROR_NULL(pSharedModel, udR_MemoryAllocationFailure);

  UD_ERROR_CHECK(udOctree_Create(&pSharedModel->pOctree, name.toStringz(), useStreamer, 0));

  spModel = UDModelRef(pSharedModel);

epilogue:

  return spModel;
}

UDModel::~UDModel()
{
  if (pOctree)
    pOctree->pDestroy(pOctree);
}

} // namespace ud

