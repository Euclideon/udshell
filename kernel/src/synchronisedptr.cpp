#include "synchronisedptr.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/delegate.h"

namespace ep {
namespace internal {

void destroyOnMainThread(Kernel *pKernel, RefCounted *pInstance)
{
  struct S
  {
    void destroy()
    {
      ((RefCounted*)this)->decRef();
    }
  };

  pKernel->dispatchToMainThread(MakeDelegate((S*)pInstance, &S::destroy));
}

} // namespace internal
} // namespace ep
