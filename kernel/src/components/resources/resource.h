#pragma once
#ifndef _EP_RESOURCE_H
#define _EP_RESOURCE_H

#include "components/component.h"

namespace ep
{

SHARED_CLASS(DataSource);
SHARED_CLASS(Metadata);
SHARED_CLASS(Resource);

class Resource : public Component
{
public:
  EP_COMPONENT(Resource);

  MetadataRef GetMetadata() const;

  epEvent<> Changed;

protected:
  Resource(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {}

  DataSourceRef source = nullptr;
  MetadataRef metadata = nullptr;
};

}

#endif // _EP_RESOURCE_H
