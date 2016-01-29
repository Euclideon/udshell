#pragma once
#if !defined(_EP_IDATASOURCE_HPP)
#define _EP_IDATASOURCE_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(Resource);
SHARED_CLASS(Stream);

class IDataSource
{
public:
  virtual Slice<const String> GetFileExtensions() const = 0;
  virtual epResult Flush() = 0; // TODO Get rid of epResult

  virtual size_t GetNumResources() const = 0;
  virtual String GetResourceName(size_t index) const = 0;
  virtual ResourceRef GetResource(size_t index) const = 0;
  virtual ResourceRef GetResource(String name) const = 0;

  virtual void SetResource(String name, const ResourceRef &spResource) = 0;
  virtual size_t CountResources(String prefix) = 0; // TODO

  virtual String GetURL() const = 0;
  virtual void SetURL(String url) = 0;

  virtual Variant Save() = 0;

protected:
  virtual StreamRef OpenStream(const Variant &source) = 0;
  virtual ResourceRef GetResourceByVariant(Variant index) const = 0;
};

}

#endif // _EP_IDATASOURCE_HPP
