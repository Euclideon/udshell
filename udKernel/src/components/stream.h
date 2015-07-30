#pragma once
#if !defined(_UD_STREAM_H)
#define _UD_STREAM_H

#include "component.h"

namespace ud
{

class Stream : public Component
{
public:
  UD_COMPONENT(Stream);

  virtual size_t Read(void *pData, size_t bytes);
  virtual size_t Write(void *pData, size_t bytes);

protected:
  Stream(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {}
};

}

#endif // _UD_STREAM_H
