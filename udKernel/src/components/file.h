#pragma once
#if !defined(_UD_FILE_H)
#define _UD_FILE_H

#include "stream.h"

namespace ud
{

class File : public Stream
{
public:
  UD_COMPONENT(File);

  size_t Read(void *pData, size_t bytes) override;
  size_t Write(void *pData, size_t bytes) override;

protected:
  File(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Stream(pType, pKernel, uid, initParams)
  {
  }
};

}

#endif // _UD_FILE_H
