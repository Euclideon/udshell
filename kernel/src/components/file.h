#pragma once
#if !defined(_EP_FILE_H)
#define _EP_FILE_H

#include "stream.h"

namespace ep
{

SHARED_CLASS(File);

EP_BITFIELD(FileOpenFlags,
  Read,
  Write,
  Create,
  Append,
  Text
);

class File : public Stream
{
  EP_DECLARE_COMPONENT(File, Stream, EPKERNEL_PLUGINVERSION, "File desc...")
public:

  Slice<void> Read(Slice<void> buffer) override;
  size_t Write(Slice<const void> data) override;
  int64_t Seek(SeekOrigin rel, int64_t offset) override;

protected:
  File(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~File();

  int GetPosixOpenFlags(FileOpenFlags flags) const;
  int fd;
};

}

#endif // _EP_FILE_H
