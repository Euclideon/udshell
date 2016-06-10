#pragma once
#if !defined(_EP_FILEIMPL_H)
#define _EP_FILEIMPL_H

#include "ep/cpp/component/file.h"
#include "ep/cpp/internal/i/ifile.h"

#include "ep/cpp/component/stream.h"

namespace ep
{

SHARED_CLASS(File);

class FileImpl : public BaseImpl<File, IFile>
{
public:
  FileImpl(Component *pInstance, Variant::VarMap initParams);
  ~FileImpl();

  Slice<void> Read(Slice<void> buffer) override final;
  size_t Write(Slice<const void> data) override final;
  int64_t Seek(SeekOrigin rel, int64_t offset) override final;

protected:
  int GetPosixOpenFlags(FileOpenFlags flags) const;
  int fd;
};

class FileImplStatic : public BaseStaticImpl<IFileStatic>
{
  MutableString<260> UrlToNativePath(String url) override final;
};

}

#endif // _EP_FILE_H
