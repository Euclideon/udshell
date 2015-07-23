#pragma once
#ifndef UDMODEL_H // Changes this name once we have a new prefix!
#define UDMODEL_H

#include "udPlatform.h"
#include "udOctree.h"

#include "udSharedPtr.h"
#include "udString.h"

SHARED_CLASS(udSharedUDModel);

class udSharedUDModel : public udRefCounted
{
public:
  static udSharedUDModelRef Create(udString name, bool useStreamer);

  udOctree *GetOctreePtr() const { return pOctree; }

protected:
  template<typename T>
  friend class udSharedPtr;

  virtual ~udSharedUDModel();

private:
  udSharedUDModel() {}

  udOctree *pOctree;
};

#endif // UDMODEL_H
