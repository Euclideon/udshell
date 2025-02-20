
#include "ep/c/plugin.h"

namespace ep {

template<typename T>
ptrdiff_t epStringify(Slice<char> buffer, String format, SharedPtr<T> spT, const VarArg *pArgs)
{
  if (!spT)
    return ::epStringify(buffer, format, nullptr, pArgs);
  return epStringifyTemplate(buffer, format, spT.get(), pArgs);
}

// cast functions
template<class T, class U>
inline UniquePtr<T> unique_pointer_cast(const UniquePtr<U> &ptr)
{
  return UniquePtr<T>((T*)ptr.get());
}
template<class T, class U>
inline SharedPtr<T> shared_pointer_cast(const SharedPtr<U> &ptr)
{
  return SharedPtr<T>(static_cast<T*>(ptr.get()));
}

namespace internal {

  struct PointerHash
  {
    static uint32_t hash(void *pAlloc);
    static bool eq(const void *a, const void *b) { return a == b; }
  };
}

} // namespace ep
