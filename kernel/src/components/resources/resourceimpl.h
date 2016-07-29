#pragma once
#ifndef _EP_RESOURCEIMPL_H
#define _EP_RESOURCEIMPL_H

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/iresource.h"
#include "ep/cpp/component/datasource/datasource.h"
namespace ep {

SHARED_CLASS(Metadata);
SHARED_CLASS(Resource);

class ResourceImpl : public BaseImpl<Resource, IResource>
{
public:
  ResourceImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
  }

  MetadataRef GetMetadata() const override final;

  Variant Save() const override final { return pInstance->Super::save(); }

protected:
  MetadataRef metadata = nullptr;
};

} // end namespace ep

#endif // _EP_RESOURCEIMPL_H
