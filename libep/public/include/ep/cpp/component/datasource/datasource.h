#pragma once
#if !defined(_EP_DATASOURCE_H)
#define _EP_DATASOURCE_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/idatasource.h"
#include "ep/cpp/component/resource/resource.h"

namespace ep {

EP_BITFIELD(DataSourceFlags,
  ReadOnly,
  WriteOnly,
  FlushImmediately,
  DeferredLoad
);

SHARED_CLASS(DataSource);
SHARED_CLASS(Stream);

class DataSource : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, DataSource, IDataSource, Component, EPKERNEL_PLUGINVERSION, "DataSource desc...", 0)

public:
  virtual Slice<const String> GetFileExtensions() const { return pImpl->GetFileExtensions(); }

  virtual String GetURL() const { return pImpl->GetURL(); }
  virtual void SetURL(String url) { pImpl->SetURL(url); }

  virtual void Flush() { pImpl->Flush(); }

  virtual size_t GetNumResources() const { return pImpl->GetNumResources(); }
  virtual String GetResourceName(size_t index) const { return pImpl->GetResourceName(index); }

  virtual ResourceRef GetResource(size_t index) const { return pImpl->GetResourceFromIndex(index); }

  virtual ResourceRef GetResource(String _name) const { return pImpl->GetResource(_name); }
  virtual void SetResource(String _name, const ResourceRef &spResource) { pImpl->SetResource(_name, spResource); }

  template<typename T>
  SharedPtr<T> GetResourceAs(size_t index) const
  {
    return component_cast<T>(GetResource(index));
  }
  template<typename T>
  SharedPtr<T> GetResourceAs(String _name) const
  {
    return component_cast<T>(GetResource(_name));
  }

  size_t CountResources(String filterPattern) const { return pImpl->CountResources(filterPattern); } // TODO

  template<typename ResType = Resource>
  ResourceRef Get(size_t resourceIndex) const
  {
    return shared_pointer_cast<ResType>(GetResource(ResType::Name(), resourceIndex));
  }

  Variant save() const override { return pImpl->Save(); }

protected:
  DataSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  virtual StreamRef OpenStream(const Variant &source) { return pImpl->OpenStream(source); }

private:
  ResourceRef GetResourceByVariant(Variant index) const
  {
    if (index.is(Variant::Type::String))
      return GetResource(index.asString());
    else
      return GetResource((size_t)index.asInt());
  }

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

}

#endif // _EP_DATASOURCE_H
