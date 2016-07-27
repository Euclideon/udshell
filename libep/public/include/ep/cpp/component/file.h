#pragma once
#ifndef EPFILE_H
#define EPFILE_H

#include "ep/cpp/internal/i/ifile.h"

#include "ep/cpp/component/stream.h"

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
  EP_DECLARE_COMPONENT_WITH_STATIC_IMPL(ep, File, IFile, IFileStatic, Stream, EPKERNEL_PLUGINVERSION, "File desc...", 0)
public:

  Slice<void> read(Slice<void> buffer) override { return pImpl->Read(buffer); }
  size_t write(Slice<const void> data) override { return pImpl->Write(data); }
  int64_t seek(SeekOrigin rel, int64_t offset) override { return pImpl->Seek(rel, offset); }

  static MutableString<260> urlToNativePath(String url) { return GetStaticImpl()->UrlToNativePath(url); }

protected:
  File(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Stream(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }
};

}

#endif // EPFILE_H
