#pragma once
#ifndef _UD_RESOURCE_H
#define _UD_RESOURCE_H

#include "components/component.h"

namespace ud
{

SHARED_CLASS(DataSource);

class Resource : public Component
{
public:
  UD_COMPONENT(Resource);

  udString Type() const { return type; }

  virtual udVariant GetMetadata(udString key) const { return udVariant(); }

protected:
  Resource(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Component(pType, pKernel, uid, initParams) {}

  udString type;
  DataSourceRef source = nullptr;
};
SHARED_CLASS(Resource);

}

#endif // _UD_RESOURCE_H
