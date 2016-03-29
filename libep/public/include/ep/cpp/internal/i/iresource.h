#pragma once
#if !defined(_EP_IRESOURCE_HPP)
#define _EP_IRESOURCE_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(DataSource);
SHARED_CLASS(Metadata);

class IResource
{
public:
  virtual MetadataRef GetMetadata() const = 0;

  virtual Variant Save() const = 0;
};

}

#endif // _EP_RESOURCE_H
