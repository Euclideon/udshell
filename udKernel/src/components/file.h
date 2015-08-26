#pragma once
#if !defined(_UD_FILE_H)
#define _UD_FILE_H

#include "stream.h"

struct udFile;

namespace ud
{

PROTOTYPE_COMPONENT(File);

class File : public Stream
{
public:
  UD_COMPONENT(File);

  size_t Read(void *pData, size_t bytes) override;
  size_t Write(const void *pData, size_t bytes) override;

  int64_t Seek(SeekOrigin rel, int64_t offset) override;

protected:
  File(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams);
  ~File();

  udFile *pFile;
};

}

#endif // _UD_FILE_H
