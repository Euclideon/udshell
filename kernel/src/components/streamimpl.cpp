#include "components/streamimpl.h"
#include "kernel.h"

namespace ep
{

BufferRef StreamImpl::ReadBuffer(size_t bytes)
{
  BufferRef spBuffer = GetKernel()->CreateComponent<Buffer>();
  if (!spBuffer)
    return nullptr;

  spBuffer->Allocate(bytes);

  Slice<void> buffer = spBuffer->Map();
  if (!buffer)
    return nullptr;

  IF_EPASSERT(Slice<void> read =) Read(buffer);
  spBuffer->Unmap();

  EPASSERT(read.length == bytes, "TODO: handle the case where we read less bytes than we expect!");

  return spBuffer;
}

BufferRef StreamImpl::Load()
{
  int64_t len = Length();
  if (len < 0)
    return nullptr;

  BufferRef spBuffer = GetKernel()->CreateComponent<Buffer>();
  if (!spBuffer)
    return nullptr;
  spBuffer->Allocate((size_t)len);

  Slice<void> buffer = spBuffer->Map();
  if (!buffer)
    return nullptr;

  Seek(SeekOrigin::Begin, 0);
  Read(buffer);
  spBuffer->Unmap();

  return spBuffer;
}

void StreamImpl::Save(BufferRef spBuffer)
{
  // TODO: check and bail if stream is not writable...

  Slice<const void> buffer = spBuffer->MapForRead();
  if (buffer)
  {
    Seek(SeekOrigin::Begin, 0);
    pInstance->Write(buffer);

    spBuffer->Unmap();
  }
}

String StreamImpl::ReadLn(Slice<char> buf)
{
  EPASSERT(false, "TODO");

  return nullptr;
}

} // namespace ep
