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

  virtual int64_t Length() const { return 0; }

  virtual size_t Read(void *pData, size_t bytes) { return 0; }
  virtual size_t Write(void *pData, size_t bytes) { return 0; }

  virtual int64_t GetPos() { return 0; }
  virtual int64_t Seek(SeekOrigin rel, int64_t offset) { return 0; }

protected:
  File(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Stream(pType, pKernel, uid, initParams)
  {
  }
};

}

#endif // _UD_FILE_H
