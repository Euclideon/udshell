#include "synchronisedptr.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/delegate.h"

namespace ep {
namespace internal {

void DestroyOnMainThread(Kernel *pKernel, RefCounted *pInstance)
{
  struct S
  {
    void Destroy(Kernel *pKernel)
    {
      ((RefCounted*)this)->DecRef();
    }
  };

  pKernel->DispatchToMainThread(fastdelegate::MakeDelegate((S*)pInstance, &S::Destroy));
}

} // namespace internal
} // namespace ep
