
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

HashMap<void*, SafeProxy<void>*, PointerHash> *GetWeakRefRegistry() {return (HashMap<void*, SafeProxy<void>*, PointerHash>*)s_pInstance->WeakRegistry(); }

void* GetSafePtr(void *pAlloc)
{
  auto pRegistry = GetWeakRefRegistry();
  if (pRegistry)
    return pRegistry->tryInsert(pAlloc, [&]() { return RefCounted::New<SafeProxy<void>>(pAlloc); });
  return nullptr;
}

void NullifySafePtr(void *pAlloc)
{
  auto pRegistry = GetWeakRefRegistry();
  if (pRegistry)
  {
    SafeProxy<void> **ppProxy = pRegistry->get(pAlloc);
    if (ppProxy)
    {
      (*ppProxy)->pInstance = nullptr;
      pRegistry->remove(pAlloc);
    }
  }
}

} // namespace internal

void RefCounted::Destroy()
{
  delete this;
}

} // namespace ep
