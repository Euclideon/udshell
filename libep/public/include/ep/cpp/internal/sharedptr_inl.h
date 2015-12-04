
#include "ep/c/plugin.h"

namespace ep {

template<typename T>
ptrdiff_t epStringify(Slice<char> buffer, String format, SharedPtr<T> spT, const epVarArg *pArgs)
{
  return epStringifyTemplate(buffer, format, spT.ptr(), pArgs);
}

// cast functions
template<class T, class U>
inline UniquePtr<T> unique_pointer_cast(const UniquePtr<U> &ptr)
{
  return UniquePtr<T>((T*)ptr.ptr());
}
template<class T, class U>
inline SharedPtr<T> shared_pointer_cast(const SharedPtr<U> &ptr)
{
  return SharedPtr<T>((T*)ptr.ptr());
}

} // namespace ep
