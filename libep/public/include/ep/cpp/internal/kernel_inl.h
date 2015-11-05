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

inline epResult Kernel::SendMessage(String target, String sender, String message, const Variant &data)
{
  return s_pPluginInstance->pKernelAPI->pSendMessage(s_pPluginInstance->pKernelInstance, target, sender, message, (epVariant*)&data);
}

inline epResult Kernel::CreateComponent(String typeId, Slice<const KeyValuePair> initParams, Component **ppNewInstance)
{
  return s_pPluginInstance->pKernelAPI->pCreateComponent(s_pPluginInstance->pKernelInstance, typeId, (const epKeyValuePair*)initParams.ptr, initParams.length, (epComponent**)ppNewInstance);
}

inline epComponent* Kernel::FindComponent(String uid) const
{
  return s_pPluginInstance->pKernelAPI->pFindComponent(s_pPluginInstance->pKernelInstance, uid);
}

inline void Kernel::Exec(String code)
{
  s_pPluginInstance->pKernelAPI->pExec(s_pPluginInstance->pKernelInstance, code);
}

template<typename ...Args>
inline void Kernel::LogError(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    s_pPluginInstance->pKernelAPI->pLogError(s_pPluginInstance->pKernelInstance, format, String());
  else
  {
    MutableString128 tmp; tmp.format(format, args...);
    s_pPluginInstance->pKernelAPI->pLogError(s_pPluginInstance->pKernelInstance, (String)tmp, String());
  }
}
template<typename ...Args>
inline void Kernel::LogWarning(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    s_pPluginInstance->pKernelAPI->pLogWarning(s_pPluginInstance->pKernelInstance, level, format, String());
  else
  {
    MutableString128 tmp; tmp.format(format, args...);
    s_pPluginInstance->pKernelAPI->pLogWarning(s_pPluginInstance->pKernelInstance, level, (String)tmp, String());
  }
}
template<typename ...Args>
inline void Kernel::LogDebug(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    s_pPluginInstance->pKernelAPI->pLogDebug(s_pPluginInstance->pKernelInstance, level, format, String());
  else
  {
    MutableString128 tmp; tmp.format(format, args...);
    s_pPluginInstance->pKernelAPI->pLogDebug(s_pPluginInstance->pKernelInstance, level, (String)tmp, String());
  }
}
template<typename ...Args>
inline void Kernel::LogInfo(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    s_pPluginInstance->pKernelAPI->pLogInfo(s_pPluginInstance->pKernelInstance, level, format, String());
  else
  {
    MutableString128 tmp; tmp.format(format, args...);
    s_pPluginInstance->pKernelAPI->pLogInfo(s_pPluginInstance->pKernelInstance, level, (String)tmp, String());
  }
}
template<typename ...Args>
inline void Kernel::LogScript(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    s_pPluginInstance->pKernelAPI->pLogScript(s_pPluginInstance->pKernelInstance, format, String());
  else
  {
    MutableString128 tmp; tmp.format(format, args...);
    s_pPluginInstance->pKernelAPI->pLogScript(s_pPluginInstance->pKernelInstance, (String)tmp, String());
  }
}
template<typename ...Args>
inline void Kernel::LogTrace(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    s_pPluginInstance->pKernelAPI->pLogTrace(s_pPluginInstance->pKernelInstance, format, String());
  else
  {
    MutableString128 tmp; tmp.format(format, args...);
    s_pPluginInstance->pKernelAPI->pLogTrace(s_pPluginInstance->pKernelInstance, (String)tmp, String());
  }
}

} // namespace ep
