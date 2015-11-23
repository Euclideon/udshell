#include "components/broadcaster.h"
#include "kernel.h"

namespace kernel {

size_t Broadcaster::WriteBuffer(BufferRef spData)
{
  Slice<const void> buffer = spData->MapForRead();
  size_t bytes = Write(buffer);
  spData->Unmap();
  return bytes;
}

size_t Broadcaster::WriteLn(String str)
{
  size_t written;

  MutableString<1024> temp(Concat, str, "\n");
  written = Write(temp);

  return written;
}

} // namespace kernel
