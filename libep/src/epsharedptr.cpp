
#include "ep/cpp/sharedptr.h"
#include "ep/cpp/avltree.h"
#include "ep/cpp/hashmap.h"

namespace ep {
namespace internal {

static HashMap<WeakProxy<RefCounted>*> s_weakRefRegistry(65536);

void* GetWeakPtr(void *pAlloc)
{
  size_t hash = HashPointer(pAlloc);
  WeakProxy<RefCounted> **ppProxy = s_weakRefRegistry.Get(hash);
  if (ppProxy)
    return *ppProxy;

  WeakProxy<RefCounted> *pProxy = new WeakProxy<RefCounted>;
  pProxy->pInstance = (RefCounted*)pAlloc;
  s_weakRefRegistry.Add(hash, pProxy);
  return pProxy;
}

void NullifyWeakPtr(void *pAlloc)
{
  size_t hash = HashPointer(pAlloc);
  WeakProxy<RefCounted> **ppProxy = s_weakRefRegistry.Get(hash);
  if (ppProxy)
  {
    (*ppProxy)->pInstance = nullptr;
    s_weakRefRegistry.Destroy(hash);
  }
}

} // namespace internal
} // namespace ep


class Test : public RefCounted
{
public:
  Test()
  {
    test = 10;
  }
  ~Test()
  {
    test = 100;
  }

  static int test;
};
int Test::test = 0;

epResult epSharedPtr_Test()
{
  auto upT1 = UniquePtr<Test>(new Test);
  EPASSERT(Test::test == 10, "!");

  UniquePtr<Test> upT2 = nullptr;
  upT2 = upT1;
  EPASSERT(upT1 == nullptr, "UniquePtr should be null!");
  SharedPtr<Test> spT1 = upT2;
  EPASSERT(!upT2, "UniquePtr should be null!");
  EPASSERT(spT1 != nullptr, "SharedPtr should be null!");
  EPASSERT(spT1.count() == 1, "spT1 has only ref");

  SharedPtr<Test> spT2 = upT1;
  EPASSERT(spT2 == nullptr, "!");

  spT2 = spT1;
  EPASSERT(spT1 == spT2, "!");
  EPASSERT(spT2.count() == 2, "!");

  spT2 = nullptr;
  EPASSERT(!spT2, "!");
  EPASSERT(spT1.count() == 1, "!");

  WeakPtr<Test> wpT1(spT1);
  EPASSERT(wpT1.ptr() == spT1.ptr(), "!");
  EPASSERT(spT1.count() == 1, "!");

  spT1 = nullptr;
  EPASSERT(wpT1 == nullptr, "!");

  wpT1 = nullptr;
  EPASSERT(!wpT1, "!");

  EPASSERT(Test::test == 100, "!");

  return epR_Success;
}
