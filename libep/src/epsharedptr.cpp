
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/safeptr.h"
#include "ep/cpp/avltree.h"
#include "ep/cpp/hashmap.h"
#include "ep/cpp/plugin.h"

namespace ep {
namespace internal {

uint32_t PointerHash::hash(void *pAlloc)
{
  return HashPointer(pAlloc); // Move this back to header once HashPointer is moved out of hashmap.h
}

HashMap<SafeProxy<void>*, void*, PointerHash> *GetWeakRefRegistry() {return (HashMap<SafeProxy<void>*, void*, PointerHash>*)s_pInstance->WeakRegistry(); }

void* GetSafePtr(void *pAlloc)
{
  auto pRegistry = GetWeakRefRegistry();
  if (pRegistry)
    return pRegistry->TryInsert(pAlloc, [&]() { return RefCounted::New<SafeProxy<void>>(pAlloc); });
  return nullptr;
}

void NullifySafePtr(void *pAlloc)
{
  auto pRegistry = GetWeakRefRegistry();
  if (pRegistry)
  {
    SafeProxy<void> **ppProxy = pRegistry->Get(pAlloc);
    if (ppProxy)
    {
      (*ppProxy)->pInstance = nullptr;
      pRegistry->Remove(pAlloc);
    }
  }
}

} // namespace internal

void RefCounted::Destroy()
{
  delete this;
}

} // namespace ep
