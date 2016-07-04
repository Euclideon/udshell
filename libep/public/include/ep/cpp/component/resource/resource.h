#pragma once
#ifndef _EP_RESOURCE_H
#define _EP_RESOURCE_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/iresource.h"

namespace ep {

SHARED_CLASS(Resource);
SHARED_CLASS(DataSource);

class Resource : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Resource, IResource, Component, EPKERNEL_PLUGINVERSION, "Base resource", 0)
public:
  MetadataRef GetMetadata() const { return pImpl->GetMetadata(); }

  Variant Save() const override { return pImpl->Save(); }

  virtual ResourceRef Clone() const { EPTHROW(Result::InvalidCall, "Not implemented!"); }

  Event<> Changed;

protected:
  Resource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
  Array<const EventInfo> GetEvents() const;
};

template <typename T, typename std::enable_if<std::is_base_of<Resource, T>::value>::type* = nullptr>
inline SharedPtr<T> CloneResource(SharedPtr<T> inst) { return component_cast<T>(inst->Clone()); }

}

#endif // _EP_RESOURCE_H
