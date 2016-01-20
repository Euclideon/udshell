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

  ComponentRef GetMetadata() const override final;
  DataSourceRef GetDataSource() const override final { return source; }

  Variant Save() const override final { return pInstance->InstanceSuper::Save(); }
protected:
  DataSourceRef source = nullptr;
  MetadataRef metadata = nullptr;
};

} // end namespace ep

#endif // _EP_RESOURCEIMPL_H
