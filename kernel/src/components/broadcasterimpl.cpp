#include "components/broadcasterimpl.h"
#include "ep/cpp/kernel.h"

namespace ep {

size_t BroadcasterImpl::WriteBuffer(BufferRef spData)
{
  Slice<const void> buffer = spData->mapForRead();
  size_t bytes = pInstance->Write(buffer);
  spData->unmap();
  return bytes;
}

size_t BroadcasterImpl::WriteLn(String str)
{
  size_t written;

  MutableString<1024> temp(Concat, str, "\n");
  written = pInstance->Write(temp);

  return written;
}

} // namespace ep
