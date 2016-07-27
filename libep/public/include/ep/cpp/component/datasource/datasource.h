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
  virtual Slice<const String> getFileExtensions() const { return pImpl->GetFileExtensions(); }

  virtual String getURL() const { return pImpl->GetURL(); }
  virtual void setURL(String url) { pImpl->SetURL(url); }

  virtual void flush() { pImpl->Flush(); }

  virtual size_t getNumResources() const { return pImpl->GetNumResources(); }
  virtual String getResourceName(size_t index) const { return pImpl->GetResourceName(index); }

  virtual ResourceRef getResource(size_t index) const { return pImpl->GetResourceFromIndex(index); }

  virtual ResourceRef getResource(String _name) const { return pImpl->GetResource(_name); }
  virtual void setResource(String _name, const ResourceRef &spResource) { pImpl->SetResource(_name, spResource); }

  template<typename T>
  SharedPtr<T> getResourceAs(size_t index) const
  {
    return component_cast<T>(getResource(index));
  }
  template<typename T>
  SharedPtr<T> getResourceAs(String _name) const
  {
    return component_cast<T>(getResource(_name));
  }

  size_t countResources(String filterPattern) const { return pImpl->CountResources(filterPattern); } // TODO

  template<typename ResType = Resource>
  ResourceRef get(size_t resourceIndex) const
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

  virtual StreamRef openStream(const Variant &source) { return pImpl->OpenStream(source); }

private:
  ResourceRef getResourceByVariant(Variant index) const
  {
    if (index.is(Variant::Type::String))
      return getResource(index.asString());
    else
      return getResource((size_t)index.asInt());
  }

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;
};

}

#endif // _EP_DATASOURCE_H
