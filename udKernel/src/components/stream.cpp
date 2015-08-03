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
  size_t read = Read(pBuffer, bytes);
  spBuffer->UnMap();

  UDASSERT(read == bytes, "TODO: handle the case where we read less bytes than we expect!");

  return spBuffer;
}
size_t Stream::WriteBuffer(BufferRef spData)
{
  size_t bytes;
  const void *pBuffer = spData->MapForRead(&bytes);
  bytes = Write(pBuffer, bytes);
  spData->UnMap();
  return bytes;
}

BufferRef Stream::Load()
{
  size_t len = Length();
  if (len < 0)
    return nullptr;

  BufferRef spBuffer = pKernel->CreateComponent<Buffer>();
  spBuffer->Allocate(len);

  void *pBuffer = spBuffer->Map();
  Seek(SeekOrigin::Begin, 0);
  Read(pBuffer, len);
  spBuffer->UnMap();

  return spBuffer;
}

void Stream::Save(BufferRef spBuffer)
{
  // TODO: check and bail if stream is not writable...

  size_t len;
  const void *pBuffer = spBuffer->MapForRead(&len);

  Seek(SeekOrigin::Begin, 0);
  Write(pBuffer, len);

  spBuffer->UnMap();
}

} // namespace ud
