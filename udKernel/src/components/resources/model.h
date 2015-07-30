#pragma once
#ifndef _UD_MODEL_H
#define _UD_MODEL_H

#include "components/resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Model);

class Model : public Resource
{
public:
  UD_COMPONENT(Model);

protected:
  Model(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Model();
};

} // namespace ud

#endif // _UD_MODEL_H
