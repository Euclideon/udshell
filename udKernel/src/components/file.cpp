#include "components/file.h"

#include "udFile.h"

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

File::File(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
  : Stream(pType, pKernel, uid, initParams)
{
  const udVariant &path = initParams["path"];
  if (!path.is(udVariant::Type::String))
    throw udR_InvalidParameter_;

  // TODO: handle open flags (somehow)

  udFileOpenFlags flags = udFOF_Read;

  udResult r = udFile_Open(&pFile, path.asString().toStringz(), flags, &length);
  if (r != udR_Success)
    throw r;
}

File::~File()
{
  udFile_Close(&pFile);
}

size_t File::Read(void *pData, size_t bytes)
{
  size_t read;
  udFile_SeekRead(pFile, pData, bytes, pos, udFSW_SeekSet, &read, &pos);
  return read;
}

size_t File::Write(const void *pData, size_t bytes)
{
  size_t written;
  udFile_SeekWrite(pFile, pData, bytes, pos, udFSW_SeekSet, &written, &pos);
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

} // namespace ud
