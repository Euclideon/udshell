#pragma once
#ifndef _EP_RESOURCE_H
#define _EP_RESOURCE_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iresource.h"

namespace ep {

SHARED_CLASS(Resource);
SHARED_CLASS(DataSource);

class Resource : public Component, public IResource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Resource, IResource, Component, EPKERNEL_PLUGINVERSION, "Base resource")
public:
  ComponentRef GetMetadata() const override { return pImpl->GetMetadata(); } // TODO Change ComponentRef to Metadata once Metadata made public
  DataSourceRef GetDataSource() const override { return pImpl->GetDataSource(); }

  Variant Save() const override { return pImpl->Save(); }

  Event<> Changed;

protected:
  Resource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties();
  static Array<const MethodInfo> GetMethods();
  static Array<const EventInfo> GetEvents();
};

}

#endif // _EP_RESOURCE_H
