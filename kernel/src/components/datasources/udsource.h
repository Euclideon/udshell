#pragma once
#if !defined(_EP_EPSOURCE_H)
#define _EP_EPSOURCE_H

#include "ep/cpp/component/datasource/datasource.h"

namespace ep
{

SHARED_CLASS(UDSource);

class UDSource : public DataSource
{
  EP_DECLARE_COMPONENT(ep, UDSource, DataSource, EPKERNEL_PLUGINVERSION, "Provides UD Data", 0)
public:

  enum class Flags : size_t
  {
    ReadOnly = 1<<0,
    WriteOnly = 1<<1,
    FlushImmediately = 1<<2,
    DeferredLoad = 1<<3,
  };

  Slice<const String> getFileExtensions() const override
  {
    return extensions;
  }

  static Slice<const String> StaticGetFileExtensions() { return extensions; }

protected:
  static const Array<const String> extensions;

  static void StaticInit(ep::Kernel *pKernel);

  UDSource(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
};

}

#endif // _EP_EPSOURCE_H
