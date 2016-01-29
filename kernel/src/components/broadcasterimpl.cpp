#include "components/broadcasterimpl.h"
#include "kernel.h"

namespace ep {

size_t BroadcasterImpl::WriteBuffer(BufferRef spData)
{
  Slice<const void> buffer = spData->MapForRead();
  size_t bytes = pInstance->Write(buffer);
  spData->Unmap();
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
