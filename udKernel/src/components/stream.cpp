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

  udSlice<void> buffer = spBuffer->Map();
  IF_UDASSERT(udSlice<void> read =) Read(buffer);
  spBuffer->Unmap();

  UDASSERT(read.length == bytes, "TODO: handle the case where we read less bytes than we expect!");

  return spBuffer;
}
size_t Stream::WriteBuffer(BufferRef spData)
{
  udSlice<const void> buffer = spData->MapForRead();
  size_t bytes = Write(buffer);
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

  udSlice<void> buffer = spBuffer->Map();
  Seek(SeekOrigin::Begin, 0);
  Read(buffer);
  spBuffer->Unmap();

  return spBuffer;
}

void Stream::Save(BufferRef spBuffer)
{
  // TODO: check and bail if stream is not writable...

  udSlice<const void> buffer = spBuffer->MapForRead();

  Seek(SeekOrigin::Begin, 0);
  Write(buffer);

  spBuffer->Unmap();
}

size_t Stream::WriteLn(udString str)
{
  size_t written;

  written = Write(str);
  written += Write(udString("\n", 1));

  return written;
}

udString Stream::ReadLn(udSlice<char> buf)
{
  UDASSERT(false, "TODO");

  return nullptr;
}

} // namespace ud
