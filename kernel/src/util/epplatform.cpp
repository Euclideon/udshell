#include "ep/epplatform.h"

namespace ep_internal
{

  epMutableString256 assertBuffer;

  void epAssertFailed(epString condition, epString message, epString file, int line)
  {
    udDebugPrintf("ASSERT FAILED : %.*s\n%.*s\n%.*s : %d", (int)condition.length, condition.ptr, (int)message.length, message.ptr, (int)file.length, file.ptr, line);
  }

}
