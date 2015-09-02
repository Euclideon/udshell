#include "components/file.h"

#if UDPLATFORM_WINDOWS
#include <io.h>
#include <share.h>
#endif

#include <fcntl.h>
#include <sys\stat.h>

namespace ud
{

ComponentDesc File::descriptor =
{
  &Stream::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "file", // id
  "File", // displayName
  "File object", // description
};

File::File(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  const udVariant &path = initParams["path"];

  if (!path.is(udVariant::Type::String))
	  throw udR_InvalidParameter_;

  const udVariant &flags = initParams["flags"];
  FileOpenFlags of = flags.as<FileOpenFlags>();

  char *fFlags = GetfopenFlags(of);
  if (!fFlags)
    throw udR_InvalidParameter_;

  int posixFlags = GetPosixOpenFlags(of);

  int fd;
#if UDPLATFORM_WINDOWS
  _sopen_s(&fd, path.asString().toStringz(), posixFlags, SH_DENYNO, S_IREAD | S_IWRITE);
#else
  fd = _open(path.asString().toStringz(), posixFlags, S_IWUSR | S_IWGRP | S_IWOTH);
#endif

  if (fd == -1)
    throw udR_File_OpenFailure;
  if (nullptr == (pFile = _fdopen(fd, fFlags)))
  {
    _close(fd);
    throw udR_File_OpenFailure;
  }

  long pos = ftell(pFile);
  fseek(pFile, 0L, SEEK_END);
  length = ftell(pFile);
  fseek(pFile, pos, SEEK_SET);
}

int File::GetPosixOpenFlags(FileOpenFlags flags) const
{
  int posixFlags = 0;

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

  return posixFlags;
}

char *File::GetfopenFlags(FileOpenFlags flags) const
{
  char *pMode;

  if ((flags & FileOpenFlags::Append) && (flags & FileOpenFlags::Read))
  {
    pMode = "a+";
  }
  else if ((flags & FileOpenFlags::Append))
  {
    pMode = "a";
  }
  else if ((flags & FileOpenFlags::Read) && (flags & FileOpenFlags::Write) && (flags & FileOpenFlags::Create))
  {
    pMode = "w+b";  // Read/write, any existing file destroyed
  }
  else if ((flags & FileOpenFlags::Read) && (flags & FileOpenFlags::Write))
  {
    pMode = "r+b"; // Read/write, but file must already exist
  }
  else if (flags & FileOpenFlags::Read)
  {
    pMode = "rb"; // Read, file must already exist
  }
  else if ((flags & FileOpenFlags::Write) || (flags & FileOpenFlags::Create))
  {
    pMode = "wb"; // Write, any existing file destroyed (Create flag treated as Write in this case)
  }
  else
  {
    pMode = nullptr;
  }

  return pMode;
}

File::~File()
{
  fclose(pFile);
}

size_t File::Read(void *pData, size_t bytes)
{
  size_t read;

  fseek(pFile, (long)pos, SEEK_SET);
  read = fread(pData, 1, bytes, pFile);
  pos = ftell(pFile);

  return read;
}

size_t File::Write(const void *pData, size_t bytes)
{
  size_t written;

  fseek(pFile, (long)pos, SEEK_SET);
  written = fwrite(pData, 1, bytes, pFile);
  pos = ftell(pFile);
  length = udMax(pos, length);

  return written;
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
  return pos;
}

int File::Flush()
{
  if(fflush(pFile))
    return -1;

  // TODO Fix error handling
  return 0;
}

} // namespace ud
