#pragma once
#if !defined(_EP_FILE_H)
#define _EP_FILE_H

#include "stream.h"

namespace ep
{

PROTOTYPE_COMPONENT(File);

EP_BITFIELD(FileOpenFlags,
  Read,
  Write,
  Create,
  Append,
  Text
);

class File : public Stream
{
public:
  EP_COMPONENT(File);

  Slice<void> Read(Slice<void> buffer) override;
  size_t Write(Slice<const void> data) override;
  int64_t Seek(SeekOrigin rel, int64_t offset) override;

protected:
  File(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  ~File();

  int GetPosixOpenFlags(FileOpenFlags flags) const;
  int fd;
};

}

#endif // _EP_FILE_H
