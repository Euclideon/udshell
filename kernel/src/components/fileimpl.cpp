#include "components/fileimpl.h"
#include "components/streamimpl.h"
#include "components/broadcasterimpl.h"
#include "ep/cpp/component/broadcaster.h"
#include "ep/cpp/kernel.h"
#if defined(EP_WINDOWS)
# pragma warning(disable:4996) //The POSIX name for this item is deprecated
# include <io.h>
# include <share.h>
# define lseek _lseeki64
#elif defined(EP_LINUX)
# include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

namespace ep {

FileImpl::FileImpl(Component *_pInstance, Variant::VarMap initParams)
  : ImplSuper(_pInstance)
{
  const Variant *path = initParams.get("path");

  if (!path || !path->is(Variant::Type::String))
	  EPTHROW_ERROR(Result::InvalidArgument, "Missing or invalid 'path'");

  const Variant *flags = initParams.get("flags");
  if (!flags)
    EPTHROW_ERROR(Result::InvalidArgument, "Missing file open 'flags'");

  FileOpenFlags of = flags->as<FileOpenFlags>();

  int posixFlags = GetPosixOpenFlags(of);

  MutableString<260> cPath = pInstance->urlToNativePath(path->asString());

#if defined(EP_WINDOWS)
  // Convert UTF-8 to UTF-16 -- TODO use UD helper functions or add some to hal?
  int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, cPath.ptr, (int)cPath.length, nullptr, 0);
  wchar_t *widePath = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, cPath.ptr, (int)cPath.length, widePath, len) == 0)
    *widePath = 0;
  widePath[len] = 0;

  _wsopen_s(&fd, widePath, posixFlags, SH_DENYNO, S_IREAD | S_IWRITE);
#else
  fd = open(cPath.toStringz(), posixFlags, S_IRUSR | S_IWUSR);
#endif
  if (fd == -1)
    EPTHROW_WARN(Result::File_OpenFailure, 2, "Failed to open {0}, flags {1}, errno {2}\n", cPath, posixFlags, errno);

  uint64_t curr = lseek(fd, 0L, SEEK_CUR);
  pInstance->Stream::setLength(lseek(fd, 0L, SEEK_END));
  lseek(fd, curr, SEEK_SET);
}

MutableString<260> FileImplStatic::UrlToNativePath(String url)
{
  MutableString<260> path;

  #if defined(EP_WINDOWS)
    const char * const pFilePrefix = "file:///";
  #else
    const char * const pFilePrefix = "file://";
  #endif // defined(EP_WINDOWS)

  String noPrefixPath = url.getRightAtFirstIC(pFilePrefix, false);
  if (!noPrefixPath.empty())
    path.urlDecode(noPrefixPath);
  else
    path = url;

  return path;
}

int FileImpl::GetPosixOpenFlags(FileOpenFlags flags) const
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

FileImpl::~FileImpl()
{
  close(fd);
}

Slice<void> FileImpl::Read(Slice<void> buffer)
{
  int nRead;

  lseek(fd, pInstance->Stream::getPos(), SEEK_SET);
  nRead = read(fd, buffer.ptr, (unsigned int)buffer.length);
  pInstance->Stream::setPos(lseek(fd, 0L, SEEK_CUR));

  if (nRead == -1)
    return nullptr;

  return buffer.slice(0, nRead);
}

size_t FileImpl::Write(Slice<const void> data)
{
  lseek(fd, pInstance->Stream::getPos(), SEEK_SET);
  int written = write(fd, data.ptr, (unsigned int)data.length);
  pInstance->Stream::setPos(lseek(fd, 0L, SEEK_CUR));
  pInstance->Stream::setLength(Max(pInstance->Stream::getPos(), pInstance->Stream::length()));
  if (written == -1)
    return 0;

  pInstance->Broadcaster::write(data);

  return (size_t)written;
}

int64_t FileImpl::Seek(SeekOrigin rel, int64_t offset)
{
  int64_t pos = pInstance->Stream::getPos();
  int64_t length = pInstance->Stream::length();

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

  pInstance->Stream::setPos(pos);
  pInstance->Stream::posChanged.signal();

  return pos;
}

} // namespace ep
