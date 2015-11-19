#include "ep/c/internal/kernel_inl.h"

namespace ep {

namespace internal {

  template <typename T>
  inline constexpr auto HasDescriptor(T* t) -> decltype(T::GetDescriptor(), bool()) { return true; }
  inline constexpr bool HasDescriptor(...) { return false; }
  template <typename T>
  inline auto GetProperties(T* t) -> decltype(T::GetProperties(), Array<const epPropertyDesc>()) { return T::GetProperties(); }
  inline Array<const epPropertyDesc> GetProperties(...) { return nullptr; }
  template <typename T>
  inline auto GetMethods(T* t) -> decltype(T::GetMethods(), Array<const epMethodDesc>()) { return T::GetMethods(); }
  inline Array<const epMethodDesc> GetMethods(...) { return nullptr; }
  template <typename T>
  inline auto GetEvents(T* t) -> decltype(T::GetEvents(), Array<const epEventDesc>()) { return T::GetEvents(); }
  inline Array<const epEventDesc> GetEvents(...) { return nullptr; }
  template <typename T>
  inline auto GetStaticFuncs(T* t) -> decltype(T::GetStaticFuncs(), Array<const epStaticFuncDesc>()) { return T::GetStaticFuncs(); }
  inline Array<const epStaticFuncDesc> GetStaticFuncs(...) { return nullptr; }

} // namespace ep

template<typename ComponentType>
inline epResult Kernel::RegisterComponentType()
{
  // check the class has a 'super' member
  static_assert(internal::HasDescriptor((ComponentType*)nullptr), "Missing descriptor: needs 'EP_COMPONENT(Name, SuperType, Version, Description)' declared at the top of ComponentType");

  // keep the data statically, so that the ownership doesn't become a problem...
  auto overrides = ComponentType::GetOverrides();

  // fabricate a CreateInstance function for this type...
  overrides.pCreateInstance = [](epComponent *pBaseInstance, const epKeyValuePair *pInitParams, size_t numInitParams) -> void*
  {
    return new ComponentType(*(Component*)pBaseInstance, Slice<const KeyValuePair>((const KeyValuePair*)pInitParams, numInitParams));
  };

  Array<const epPropertyDesc> properties = internal::GetProperties((ComponentType*)nullptr);
  Array<const epMethodDesc> methods = internal::GetMethods((ComponentType*)nullptr);
  Array<const epEventDesc> events = internal::GetEvents((ComponentType*)nullptr);
  Array<const epStaticFuncDesc> staticfuncs = internal::GetStaticFuncs((ComponentType*)nullptr);

  epComponentDesc desc;
  desc.info = ComponentType::GetDescriptor();
  desc.baseClass = ComponentType::Super::GetDescriptor().id;
  desc.pOverrides = &overrides;
  desc.numProperties = properties.length;
  desc.pProperties = properties.ptr;
  desc.numMethods = methods.length;
  desc.pMethods = methods.ptr;
  desc.numEvents = events.length;
  desc.pEvents = events.ptr;
  desc.numStaticFuncs = staticfuncs.length;
  desc.pStaticFuncs = staticfuncs.ptr;

  return s_pPluginInstance->pKernelAPI->pRegisterComponentType((epKernel*)this, &desc);
}


// Inlines pipe through C API

inline Kernel* Kernel::GetInstance()
{
  return (Kernel*)s_pPluginInstance->pKernelInstance;
}

template<typename ...Args>
inline void Kernel::LogError(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(0, 2, format);
  else
    Log(0, 2, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogWarning(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(1, level, format);
  else
    Log(1, level, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogDebug(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(2, level, format);
  else
    Log(2, level, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogInfo(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(3, level, format);
  else
    Log(3, level, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogScript(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(4, 2, format);
  else
    Log(4, 2, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogTrace(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(5, 2, format);
  else
    Log(5, 2,MutableString128(Format, format, args...));
}

} // namespace ep
