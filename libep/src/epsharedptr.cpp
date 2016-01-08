
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/safeptr.h"
#include "ep/cpp/avltree.h"
#include "ep/cpp/hashmap.h"

namespace ep {
namespace internal {

struct PointerHash
{
  static uint32_t hash(void *pAlloc) { return HashPointer(pAlloc); }
  static bool eq(const void *a, const void *b) { return a == b; }
};

static HashMap<SafeProxy<void>*, void*, PointerHash> s_weakRefRegistry(65536);

void* GetSafePtr(void *pAlloc)
{
  return *s_weakRefRegistry.InsertLazy(pAlloc, [&]() { return new SafeProxy<void>(pAlloc); });
}

void NullifySafePtr(void *pAlloc)
{
  SafeProxy<void> **ppProxy = s_weakRefRegistry.Get(pAlloc);
  if (ppProxy)
  {
    (*ppProxy)->pInstance = nullptr;
    s_weakRefRegistry.Remove(pAlloc);
  }
}

} // namespace internal
} // namespace ep
