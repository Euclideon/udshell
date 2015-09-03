#pragma once
#if !defined(_UD_UDSOURCE_H)
#define _UD_UDSOURCE_H

#include "components/datasource.h"

namespace ud
{

PROTOTYPE_COMPONENT(UDDataSource);

class UDDataSource : public DataSource
{
public:
  UD_COMPONENT(UDDataSource);

  enum class Flags : size_t
  {
    ReadOnly = 1<<0,
    WriteOnly = 1<<1,
    FlushImmediately = 1<<2,
    DeferredLoad = 1<<3,
  };

  udSlice<const udString> GetFileExtensions() const override
  {
    return {".uds", ".ssf", ".upc", ".udi", ".oct3" };
  }

protected:
  UDDataSource(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
};

}

#endif // _UD_UDSOURCE_H
