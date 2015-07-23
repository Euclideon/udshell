#pragma once
#ifndef UDMODEL_H // Changes this name once we have a new prefix!
#define UDMODEL_H

#include "udPlatform.h"
#include "udSharedPtr.h"
#include "udOctree.h"
#include "udString.h"

class udSharedudModel : public udRefCounted
{
public:
  static udSharedPtr<udSharedudModel> Create(udString name, bool useStreamer);
  udOctree *GetOctreePtr() { return pOctree; }

protected:
  virtual ~udSharedudModel();

private:
  udSharedudModel() {}
  udOctree *pOctree;
};

#endif // UDMODEL_H
