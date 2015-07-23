#include "udModel.h"

udSharedUDModelRef udSharedUDModel::Create(udString name, bool useStreamer)
{
  udResult result;
  udSharedUDModelRef spModel(nullptr);

  udSharedUDModel *pSharedModel = udNew(udSharedUDModel);
  UD_ERROR_NULL(pSharedModel, udR_MemoryAllocationFailure);

  UD_ERROR_CHECK(udOctree_Create(&pSharedModel->pOctree, name.toStringz(), useStreamer, 0));

  spModel = udSharedUDModelRef(pSharedModel);

epilogue:

  return spModel;
}
