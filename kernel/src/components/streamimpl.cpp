#include "components/streamimpl.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/text.h"

namespace ep
{

BufferRef StreamImpl::ReadBuffer(size_t bytes)
{
  BufferRef spBuffer = GetKernel()->createComponent<Buffer>();
  spBuffer->allocate(bytes);

  Slice<void> buffer = spBuffer->map();
  if (!buffer)
    return nullptr;
  epscope(exit) { spBuffer->unmap(); };

  IF_EPASSERT(Slice<void> read =) pInstance->Read(buffer);
  EPASSERT(read.length == bytes, "TODO: handle the case where we read less bytes than we expect!");

  return spBuffer;
}

BufferRef StreamImpl::Load()
{
  int64_t len = pInstance->Length();
  if (len < 0)
    return nullptr;

  BufferRef spBuffer = GetKernel()->createComponent<Buffer>();
  spBuffer->allocate((size_t)len);

  Slice<void> buffer = spBuffer->map();
  if (!buffer)
    return nullptr;
  epscope(exit) { spBuffer->unmap(); };

  pInstance->Seek(SeekOrigin::Begin, 0);
  pInstance->Read(buffer);

  return spBuffer;
}

TextRef StreamImpl::LoadText()
{
  int64_t len = pInstance->Length();
  if (len < 0)
    return nullptr;

  len += 1; // for null terminator    TODO: Text::Allocate() should override to do this implicitly

  TextRef spBuffer = GetKernel()->createComponent<Text>();
  spBuffer->allocate((size_t)len);

  Slice<char> buffer = spBuffer->map();
  if (!buffer)
    return nullptr;

  size_t bytesRead = 0;
  {
    epscope(exit) { spBuffer->unmap(); };

    pInstance->Seek(SeekOrigin::Begin, 0);
    bytesRead = pInstance->Read(buffer).length;
    if (bytesRead < (size_t)len)
      memset(buffer.ptr + bytesRead, 0, (size_t)len - bytesRead);
  }
  spBuffer->resize(bytesRead);

  return spBuffer;
}

void StreamImpl::SaveBuffer(BufferRef spBuffer)
{
  // TODO: check and bail if stream is not writable...

  Slice<const void> buffer = spBuffer->mapForRead();
  if (!buffer)
    return;
  epscope(exit) { spBuffer->unmap(); };

  pInstance->Seek(SeekOrigin::Begin, 0);
  pInstance->Write(buffer);
}

String StreamImpl::ReadLn(Slice<char> buf)
{
  EPASSERT(false, "TODO");

  return nullptr;
}

} // namespace ep
