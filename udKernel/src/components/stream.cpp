#include "components/stream.h"
#include "kernel.h"

namespace ud
{

ComponentDesc Stream::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "stream", // id
  "Stream", // displayName
  "Data stream", // description
};

BufferRef Stream::ReadBuffer(size_t bytes)
{
  BufferRef spBuffer = pKernel->CreateComponent<Buffer>();
  spBuffer->Allocate(bytes);

  void *pBuffer = spBuffer->Map();
  IF_UDASSERT(size_t read =) Read(pBuffer, bytes);
  spBuffer->Unmap();

  UDASSERT(read == bytes, "TODO: handle the case where we read less bytes than we expect!");

  return spBuffer;
}
size_t Stream::WriteBuffer(BufferRef spData)
{
  size_t bytes;
  const void *pBuffer = spData->MapForRead(&bytes);
  bytes = Write(pBuffer, bytes);
  spData->Unmap();
  return bytes;
}

BufferRef Stream::Load()
{
  int64_t len = Length();
  if (len < 0)
    return nullptr;

  BufferRef spBuffer = pKernel->CreateComponent<Buffer>();
  spBuffer->Allocate((size_t)len);

  void *pBuffer = spBuffer->Map();
  Seek(SeekOrigin::Begin, 0);
  Read(pBuffer, (size_t)len);
  spBuffer->Unmap();

  return spBuffer;
}

void Stream::Save(BufferRef spBuffer)
{
  // TODO: check and bail if stream is not writable...

  size_t len;
  const void *pBuffer = spBuffer->MapForRead(&len);

  Seek(SeekOrigin::Begin, 0);
  Write(pBuffer, len);

  spBuffer->Unmap();
}

size_t Stream::WriteLn(udString str)
{
  size_t written;

  written = Write(str.ptr, str.length);
  written += Write("\n", 1);

  return written;
}

udString Stream::ReadLn(udSlice<char> buf)
{
  UDASSERT(false, "TODO");

  return nullptr;
}

} // namespace ud
