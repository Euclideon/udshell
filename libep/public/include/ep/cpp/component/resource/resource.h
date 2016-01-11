#pragma once
#ifndef _EP_RESOURCE_H
#define _EP_RESOURCE_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iresource.h"

namespace ep {

SHARED_CLASS(Resource);

class Resource : public Component, public IResource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Resource, IResource, Component, EPKERNEL_PLUGINVERSION, "Base resource")
public:
  ComponentRef GetMetadata() const override { return pImpl->GetMetadata(); } // TODO Change ComponentRef to Metadata once Metadata made public
  ComponentRef GetDataSource() const override { return pImpl->GetDataSource(); } // TODO Change ComponentRef to DataSourceRef once DataSource made public

  Event<> Changed;

protected:
  Resource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(Metadata, "Number of records in the metadata", nullptr, 0),
      EP_MAKE_PROPERTY_RO(DataSource, "The DataSource containing this Resource", nullptr, 0),
    };
  }
  static Array<const EventInfo> GetEvents()
  {
    return{
      EP_MAKE_EVENT(Changed, "The Resource has been modified"),
    };
  }
};

}

#endif // _EP_RESOURCE_H
