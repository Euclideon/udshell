#include "udModel.h"

udSharedPtr<udSharedudModel> udSharedudModel::Create(udString name, bool useStreamer)
{
  udResult result;
  udSharedPtr<udSharedudModel> spModel(nullptr);

  udSharedudModel *pSharedModel = udNew(udSharedudModel);
  UD_ERROR_NULL(pSharedModel, udR_MemoryAllocationFailure);

  UD_ERROR_CHECK(udOctree_Create(&pSharedModel->pOctree, name.toStringz(), useStreamer, 0));

  spModel = udSharedPtr<udSharedudModel>(pSharedModel);

epilogue:

  return spModel;
}
