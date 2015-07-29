#pragma once
#ifndef _UD_UDMODEL_H
#define _UD_UDMODEL_H

#include "udPlatform.h"
#include "udOctree.h"

#include "resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(UDModel);

class UDModel : public Resource
{
public:
  static UDModelRef Create(udString name, bool useStreamer);

  udOctree *GetOctreePtr() const { return pOctree; }

protected:
  template<typename T>
  friend class udSharedPtr;

private:
  UDModel() : Resource(ResourceType::UD) {};
  virtual ~UDModel();

  udOctree *pOctree;
};

} // namespace ud

#endif // _UD_UDMODEL_H
