#include "udmodel.h"

namespace ud
{

SharedUDModelRef SharedUDModel::Create(udString name, bool useStreamer)
{
  udResult result;
  SharedUDModelRef spModel(nullptr);

  SharedUDModel *pSharedModel = udNew(SharedUDModel);
  UD_ERROR_NULL(pSharedModel, udR_MemoryAllocationFailure);

  UD_ERROR_CHECK(udOctree_Create(&pSharedModel->pOctree, name.toStringz(), useStreamer, 0));

  spModel = SharedUDModelRef(pSharedModel);

epilogue:

  return spModel;
}

SharedUDModel::~SharedUDModel()
{
  if (pOctree)
    pOctree->pDestroy(pOctree);
}
} // namespace ud

