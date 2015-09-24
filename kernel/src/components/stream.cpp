#include "components/stream.h"
#include "kernel.h"

namespace ep
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

  epSlice<void> buffer = spBuffer->Map();
  IF_UDASSERT(epSlice<void> read =) Read(buffer);
  spBuffer->Unmap();

  EPASSERT(read.length == bytes, "TODO: handle the case where we read less bytes than we expect!");

  return spBuffer;
}
size_t Stream::WriteBuffer(BufferRef spData)
{
  epSlice<const void> buffer = spData->MapForRead();
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

  epSlice<void> buffer = spBuffer->Map();
  Seek(SeekOrigin::Begin, 0);
  Read(buffer);
  spBuffer->Unmap();

  return spBuffer;
}

void Stream::Save(BufferRef spBuffer)
{
  // TODO: check and bail if stream is not writable...

  epSlice<const void> buffer = spBuffer->MapForRead();

  Seek(SeekOrigin::Begin, 0);
  Write(buffer);

  spBuffer->Unmap();
}

size_t Stream::WriteLn(epString str)
{
  size_t written;

  epMutableString<256> temp;
  temp = str;
  temp.append("\n");
  written = Write(temp);

  return written;
}

epString Stream::ReadLn(epSlice<char> buf)
{
  EPASSERT(false, "TODO");

  return nullptr;
}

} // namespace ep
