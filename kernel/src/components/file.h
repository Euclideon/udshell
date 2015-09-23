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
  Append,
  Text
);

class File : public Stream
{
public:
  UD_COMPONENT(File);

  udSlice<void> Read(udSlice<void> buffer) override;
  size_t Write(udSlice<const void> data) override;
  int64_t Seek(SeekOrigin rel, int64_t offset) override;

protected:
  File(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  ~File();

  int GetPosixOpenFlags(FileOpenFlags flags) const;
  int fd;
};

}

#endif // _UD_FILE_H
