#include "components/streamimpl.h"
#include "ep/cpp/kernel.h"

namespace ep
{

BufferRef StreamImpl::ReadBuffer(size_t bytes)
{
  BufferRef spBuffer = GetKernel()->CreateComponent<Buffer>();
  spBuffer->Allocate(bytes);

  Slice<void> buffer = spBuffer->Map();
  if (!buffer)
    return nullptr;
  epscope(exit) { spBuffer->Unmap(); };

  IF_EPASSERT(Slice<void> read =) pInstance->Read(buffer);
  EPASSERT(read.length == bytes, "TODO: handle the case where we read less bytes than we expect!");

  return spBuffer;
}

BufferRef StreamImpl::Load()
{
  int64_t len = pInstance->Length();
  if (len < 0)
    return nullptr;

  BufferRef spBuffer = GetKernel()->CreateComponent<Buffer>();
  spBuffer->Allocate((size_t)len);

  Slice<void> buffer = spBuffer->Map();
  if (!buffer)
    return nullptr;
  epscope(exit) { spBuffer->Unmap(); };

  pInstance->Seek(SeekOrigin::Begin, 0);
  pInstance->Read(buffer);

  return spBuffer;
}

void StreamImpl::Save(BufferRef spBuffer)
{
  // TODO: check and bail if stream is not writable...

  Slice<const void> buffer = spBuffer->MapForRead();
  if (!buffer)
    return;
  epscope(exit) { spBuffer->Unmap(); };

  pInstance->Seek(SeekOrigin::Begin, 0);
  pInstance->Write(buffer);
}

String StreamImpl::ReadLn(Slice<char> buf)
{
  EPASSERT(false, "TODO");

  return nullptr;
}

} // namespace ep
