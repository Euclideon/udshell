#pragma once
#ifndef _UD_RESOURCE_H
#define _UD_RESOURCE_H

#include "components/component.h"

namespace ud
{

SHARED_CLASS(DataSource);
SHARED_CLASS(Metadata);
SHARED_CLASS(Resource);

class Resource : public Component
{
public:
  UD_COMPONENT(Resource);

  MetadataRef GetMetadata() const;

  udEvent<> Changed;

protected:
  Resource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {}

  DataSourceRef source = nullptr;
  MetadataRef metadata = nullptr;
};

}

#endif // _UD_RESOURCE_H
