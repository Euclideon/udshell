#include "components/stream.h"
#include "kernel.h"

namespace kernel
{

BufferRef Stream::ReadBuffer(size_t bytes)
{
  BufferRef spBuffer = GetKernel().CreateComponent<Buffer>();
  spBuffer->Allocate(bytes);

  Slice<void> buffer = spBuffer->Map();
  IF_UDASSERT(Slice<void> read =) Read(buffer);
  spBuffer->Unmap();

  EPASSERT(read.length == bytes, "TODO: handle the case where we read less bytes than we expect!");

  return spBuffer;
}
size_t Stream::WriteBuffer(BufferRef spData)
{
  Slice<const void> buffer = spData->MapForRead();
  size_t bytes = Write(buffer);
  spData->Unmap();
  return bytes;
}

BufferRef Stream::Load()
{
  int64_t len = Length();
  if (len < 0)
    return nullptr;

  BufferRef spBuffer = GetKernel().CreateComponent<Buffer>();
  spBuffer->Allocate((size_t)len);

  Slice<void> buffer = spBuffer->Map();
  Seek(SeekOrigin::Begin, 0);
  Read(buffer);
  spBuffer->Unmap();

  return spBuffer;
}

void Stream::Save(BufferRef spBuffer)
{
  // TODO: check and bail if stream is not writable...

  Slice<const void> buffer = spBuffer->MapForRead();

  Seek(SeekOrigin::Begin, 0);
  Write(buffer);

  spBuffer->Unmap();
}

size_t Stream::WriteLn(String str)
{
  size_t written;

  MutableString<256> temp;
  temp = str;
  temp.append("\n");
  written = Write(temp);

  return written;
}

String Stream::ReadLn(Slice<char> buf)
{
  EPASSERT(false, "TODO");

  return nullptr;
}

} // namespace kernel
