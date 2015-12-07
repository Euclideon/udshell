
namespace ep {

template<class T, class U>
inline SafePtr<T> safe_pointer_cast(const SafePtr<U> &ptr)
{
  return SafePtr<T>((T*)ptr.ptr());
}

} // namespace ep
