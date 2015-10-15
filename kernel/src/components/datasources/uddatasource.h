#pragma once
#if !defined(_EP_EPSOURCE_H)
#define _EP_EPSOURCE_H

#include "components/datasource.h"

namespace ep
{

PROTOTYPE_COMPONENT(UDDataSource);

class UDDataSource : public DataSource
{
public:
  EP_COMPONENT(UDDataSource);

  enum class Flags : size_t
  {
    ReadOnly = 1<<0,
    WriteOnly = 1<<1,
    FlushImmediately = 1<<2,
    DeferredLoad = 1<<3,
  };

  Slice<const String> GetFileExtensions() const override
  {
    return extensions;
  }

protected:
  static const Array<const String> extensions;

  static epResult RegisterExtensions(Kernel *pKernel);

  UDDataSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
};

}

#endif // _EP_EPSOURCE_H
