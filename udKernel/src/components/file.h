#pragma once
#if !defined(_UD_FILE_H)
#define _UD_FILE_H

#include "stream.h"

namespace ud
{

PROTOTYPE_COMPONENT(File);

UD_BITFIELD(FileOpenFlags,
  Read,
  Write,
  Create,
  Append
);

class File : public Stream
{
public:
  UD_COMPONENT(File);

  size_t Read(void *pData, size_t bytes) override;
  size_t Write(const void *pData, size_t bytes) override;
  int64_t Seek(SeekOrigin rel, int64_t offset) override;
  int Flush() override;

protected:
  File(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  ~File();

  char *GetfopenFlags(FileOpenFlags flags) const;
  int GetPosixOpenFlags(FileOpenFlags flags) const;

  FILE *pFile;
};

}

#endif // _UD_FILE_H
