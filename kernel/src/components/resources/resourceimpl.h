#pragma once
#ifndef _EP_RESOURCEIMPL_H
#define _EP_RESOURCEIMPL_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/iresource.h"

namespace ep {

SHARED_CLASS(DataSource);
SHARED_CLASS(Metadata);
SHARED_CLASS(Resource);

class ResourceImpl : public BaseImpl<Resource, IResource>
{
public:
  ResourceImpl(Component *pInstance, Variant::VarMap initParams)
    : Super(pInstance)
  {
  }

  ComponentRef GetMetadata() const;
  ComponentRef GetDataSource() const { return source; }

protected:
  DataSourceRef source = nullptr;
  MetadataRef metadata = nullptr;
};

} // end namespace ep

#endif // _EP_RESOURCEIMPL_H
