#pragma once
#ifndef _EP_RESOURCE_H
#define _EP_RESOURCE_H

#include "ep/cpp/component.h"

namespace ep
{

SHARED_CLASS(DataSource);
SHARED_CLASS(Metadata);
SHARED_CLASS(Resource);

class Resource : public Component
{
  EP_DECLARE_COMPONENT(Resource, Component, EPKERNEL_PLUGINVERSION, "Base resource")
public:

  MetadataRef GetMetadata() const;
  DataSourceRef GetDataSource() const { return source; }

  Event<> Changed;

protected:
  Resource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {}

  DataSourceRef source = nullptr;
  MetadataRef metadata = nullptr;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(Metadata, "Number of records in the metadata", nullptr, 0),
    };
  }
};

}

#endif // _EP_RESOURCE_H
