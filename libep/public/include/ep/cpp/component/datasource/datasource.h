#pragma once
#if !defined(_EP_DATASOURCE_H)
#define _EP_DATASOURCE_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/idatasource.h"
#include "ep/cpp/component/resource/resource.h"

namespace ep {

EP_ENUM(DataSourceFlags,
  ReadOnly = 1 << 0,
  WriteOnly = 1 << 1,
  FlushImmediately = 1 << 2,
  DeferredLoad = 1 << 3
);

SHARED_CLASS(DataSource);
SHARED_CLASS(Stream);

class DataSource : public Component, public IDataSource
{
  EP_DECLARE_COMPONENT_WITH_IMPL(DataSource, IDataSource, Component, EPKERNEL_PLUGINVERSION, "DataSource desc...", 0)

public:
  Slice<const String> GetFileExtensions() const override { return pImpl->GetFileExtensions(); }
  void Flush() override { pImpl->Flush(); }
  size_t GetNumResources() const override { return pImpl->GetNumResources(); }
  String GetResourceName(size_t index) const override { return pImpl->GetResourceName(index); }
  ResourceRef GetResource(size_t index) const override { return pImpl->GetResource(index); }
  ResourceRef GetResource(String _name) const override { return pImpl->GetResource(_name); }

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

  void SetResource(String _name, const ResourceRef &spResource) override { pImpl->SetResource(_name, spResource); }
  size_t CountResources(String prefix) override { return pImpl->CountResources(prefix); } // TODO

  template<typename ResType = Resource>
  ResourceRef Get(size_t resourceIndex) const
  {
    return shared_pointer_cast<ResType>(GetResource(ResType::Name(), resourceIndex));
  }

  String GetURL() const override { return pImpl->GetURL(); }
  void SetURL(String url) override { pImpl->SetURL(url); }

  Variant Save() override { return pImpl->Save(); }

protected:
  DataSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  StreamRef OpenStream(const Variant &source) override { return pImpl->OpenStream(source); }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(NumResources, "Number of resources the data source provides", nullptr, 0),
      EP_MAKE_PROPERTY(URL, "The file or location of the DataSource", nullptr, 0),
      EP_MAKE_PROPERTY_RO(FileExtensions, "The file extensions associated with the DataSource", nullptr, 0),
    };
  }
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(GetResourceName, "Get a resource name by index"),
      EP_MAKE_METHOD(GetResourceByVariant, "Get a resource by name or index"),
      EP_MAKE_METHOD(SetResource, "Set a resource by name"),
      EP_MAKE_METHOD(CountResources, "Count resources with a name prefix"),
    };
  }

private:
  // HACK: this should be private!!
  ResourceRef GetResourceByVariant(Variant index) const override { return pImpl->GetResourceByVariant(index); }
};

}

#endif // _EP_DATASOURCE_H
