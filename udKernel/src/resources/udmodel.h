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
  UD_COMPONENT(UDModel);

  void Load(udString name, bool useStreamer);

  udOctree *GetOctreePtr() const { return pOctree; }

protected:
  template<typename T>
  friend class udSharedPtr;

private:
  UDModel(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~UDModel();

  udOctree *pOctree;
};

} // namespace ud

#endif // _UD_UDMODEL_H
