#pragma once
#ifndef UDMODEL_H // Changes this name once we have a new prefix!
#define UDMODEL_H

#include "udPlatform.h"
#include "udOctree.h"

#include "udsharedptr.h"
#include "udstring.h"

namespace ud
{

SHARED_CLASS(SharedUDModel);

class SharedUDModel : public RefCounted
{
public:
  static SharedUDModelRef Create(udString name, bool useStreamer);

  udOctree *GetOctreePtr() const { return pOctree; }

protected:
  template<typename T>
  friend class SharedPtr;

  virtual ~SharedUDModel();

private:
  SharedUDModel() {}

  udOctree *pOctree;
};

} // namespace ud

#endif // UDMODEL_H
