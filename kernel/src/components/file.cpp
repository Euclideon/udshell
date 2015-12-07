#include "components/file.h"
#include "components/broadcaster.h"

#if defined(EP_WINDOWS)
#include <io.h>
#include <share.h>
#define close _close
#define read _read
#define write _write
#define lseek _lseeki64
#endif

#include <fcntl.h>
#include <sys/stat.h>

namespace kernel
{

File::File(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  const Variant *path = initParams.Get("path");

  if (!path || !path->is(Variant::Type::String))
	  throw epR_InvalidParameter;

  const Variant *flags = initParams.Get("flags");
  FileOpenFlags of = flags->as<FileOpenFlags>();

  int posixFlags = GetPosixOpenFlags(of);

#if defined(EP_WINDOWS)
  // Convert UTF-8 to UTF-16 -- TODO use UD helper functions or add some to hal?
  String cPath = path->asString();
  int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, cPath.ptr, (int)cPath.length, nullptr, 0);
  wchar_t *widePath = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, cPath.ptr, (int)cPath.length, widePath, len) == 0)
    *widePath = 0;
  widePath[len] = 0;

  _wsopen_s(&fd, widePath, posixFlags, SH_DENYNO, S_IREAD | S_IWRITE);
#else
  fd = open(path->asString().toStringz(), posixFlags, S_IWUSR | S_IWGRP | S_IWOTH);
#endif
  if (fd == -1)
    throw epR_File_OpenFailure;

  uint64_t curr = lseek(fd, 0L, SEEK_CUR);
  length = lseek(fd, 0L, SEEK_END);
  lseek(fd, curr, SEEK_SET);
}

int File::GetPosixOpenFlags(FileOpenFlags flags) const
{
  int posixFlags = 0;

#if defined(EP_WINDOWS)
  if (!(flags & FileOpenFlags::Text))
  {
    posixFlags |= O_BINARY;
  }
#endif

  if ((flags & FileOpenFlags::Read) && (flags & FileOpenFlags::Write))
  {
    posixFlags |= O_RDWR;
  }
  else if (flags & FileOpenFlags::Read)
  {
    posixFlags |= O_RDONLY;
  }
  else if (flags & FileOpenFlags::Write)
  {
    posixFlags |= O_WRONLY;
  }

  if (flags & FileOpenFlags::Create)
  {
    posixFlags |= O_CREAT;
  }

  if (flags & FileOpenFlags::Append)
  {
    posixFlags |= O_APPEND;
  }
  else if (flags & FileOpenFlags::Write)
  {
    posixFlags |= O_TRUNC;
  }

  return posixFlags;
}

File::~File()
{
  close(fd);
}

Slice<void> File::Read(Slice<void> buffer)
{
  int nRead;

  lseek(fd, pos, SEEK_SET);
  nRead = read(fd, buffer.ptr, (unsigned int)buffer.length);
  pos = lseek(fd, 0L, SEEK_CUR);

  if (nRead == -1)
    return nullptr;

  return buffer.slice(0, nRead);
}

size_t File::Write(Slice<const void> data)
{
  lseek(fd, pos, SEEK_SET);
  int written = write(fd, data.ptr, (unsigned int)data.length);
  pos = lseek(fd, 0L, SEEK_CUR);
  length = Max(pos, length);
  if (written == -1)
    return 0;

  Broadcaster::Write(data);

  return (size_t)written;
}

int64_t File::Seek(SeekOrigin rel, int64_t offset)
{
  switch (rel)
  {
    case SeekOrigin::Begin:
      pos = offset;
      break;
    case SeekOrigin::CurrentPos:
      pos += offset;
      break;
    case SeekOrigin::End:
      if (length >= 0)
        pos = length + offset;
      else
        return -1;
      break;
  }

  PosChanged.Signal();

  return pos;
}

} // namespace kernel
