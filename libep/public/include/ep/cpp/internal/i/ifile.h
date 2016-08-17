#pragma once
#if !defined(_EP_IFILE_HPP)
#define _EP_IFILE_HPP

#include "ep/cpp/component/stream.h"

namespace ep {

SHARED_CLASS(File);

class IFile
{
public:
  virtual Slice<void> Read(Slice<void> buffer) = 0;
  virtual size_t Write(Slice<const void> data) = 0;
  virtual int64_t Seek(SeekOrigin rel, int64_t offset) = 0;
};

class IFileStatic
{
public:
  virtual MutableString<260> UrlToNativePath(String url) = 0;
};

}

#endif // _EP_IFILE_HPP
