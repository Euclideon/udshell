#include "ep/cpp/plugin.h"

namespace ep {

namespace internal {
  using ComponentDesc_InitComponentPtr = ComponentDesc::InitComponent*;
  using ComponentDesc_CreateInstanceCallbackPtr = ComponentDesc::CreateInstanceCallback*;
}

// TODO: HAX HAX! this whole mess needs to be made private somehow...
template<typename ComponentType, typename Impl>
struct Kernel::CreateHelper
{
  using CreateFunc = ComponentDesc::CreateImplCallback*;

  static constexpr bool HasDescriptor() { return HasDescriptorImpl((ComponentType*)nullptr); }
  static Array<const PropertyInfo> GetProperties() { return GetPropertiesImpl((ComponentType*)nullptr); }
  static Array<const MethodInfo> GetMethods() { return GetMethodsImpl((ComponentType*)nullptr); }
  static Array<const EventInfo> GetEvents() { return GetEventsImpl((ComponentType*)nullptr); }
  static Array<const StaticFuncInfo> GetStaticFuncs() { return GetStaticFuncsImpl((ComponentType*)nullptr); }
  static ComponentDesc::InitComponent* GetStaticInit() { return GetStaticInitImpl((ComponentType*)nullptr); }

  static ComponentDesc::CreateInstanceCallback* GetCreateFunc() { return GetCreateFuncImpl((ComponentType*)nullptr); }

  static SharedString GetSuper() { return GetSuperImpl((typename ComponentType::Super*)nullptr); }

  static CreateFunc GetCreateImpl() { return GetCreateImplImpl((typename ComponentType::Impl*)nullptr); }

private:
  // TODO: these need to go somewhere else, instantiating this class for these functions instantiates the other functions too that should be specialised for IComponent and friends
  template <typename T>
  static constexpr auto HasDescriptorImpl(T* t) -> decltype(T::MakeDescriptor(), bool()) { return true; }
  static constexpr bool HasDescriptorImpl(...) { return false; }
  template <typename T>
  static auto GetPropertiesImpl(T* t) -> decltype(T::GetProperties(), Array<const PropertyInfo>()) { return T::GetProperties(); }
  static Array<const PropertyInfo> GetPropertiesImpl(...) { return nullptr; }
  template <typename T>
  static auto GetMethodsImpl(T* t) -> decltype(T::GetMethods(), Array<const MethodInfo>()) { return T::GetMethods(); }
  static Array<const MethodInfo> GetMethodsImpl(...) { return nullptr; }
  template <typename T>
  static auto GetEventsImpl(T* t) -> decltype(T::GetEvents(), Array<const EventInfo>()) { return T::GetEvents(); }
  static Array<const EventInfo> GetEventsImpl(...) { return nullptr; }
  template <typename T>
  static auto GetStaticFuncsImpl(T* t) -> decltype(T::GetStaticFuncs(), Array<const StaticFuncInfo>()) { return T::GetStaticFuncs(); }
  static Array<const StaticFuncInfo> GetStaticFuncsImpl(...) { return nullptr; }
  template <typename T>
  static auto GetStaticInitImpl(T* t) -> decltype(T::StaticInit(nullptr), internal::ComponentDesc_InitComponentPtr()) { return &T::StaticInit; }
  static ComponentDesc::InitComponent* GetStaticInitImpl(...) { return nullptr; }

  template <typename T, typename std::enable_if<!std::is_same<T, void>::value>::type* = nullptr>
  static SharedString GetSuperImpl(T* t) { return T::ComponentID(); }
  static SharedString GetSuperImpl(...) { return nullptr; }

  template <typename T, typename std::enable_if<!std::is_same<T, void>::value>::type* = nullptr>
  static CreateFunc GetCreateImplImpl(T* t) { return [](Component *pInstance, Variant::VarMap initParams) -> void* { return new Impl(pInstance, initParams); }; }
  static CreateFunc GetCreateImplImpl(...) { return nullptr; }

  template <typename T>
  static auto GetCreateFuncImpl(T* t) -> decltype(T::CreateInstance(), internal::ComponentDesc_CreateInstanceCallbackPtr()) { return &T::CreateInstance; }
  static ComponentDesc::CreateInstanceCallback* GetCreateFuncImpl(...)
  {
    return [](const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams) -> Component* {
      MutableString128 t(Format, "New: {0} - {1}", pType->info.id, uid);
      pKernel->LogDebug(4, t);
      // TODO: this new can't exist in the wild... need to call back into kernel!!
      ComponentType *ptr = new(epAlloc(sizeof(ComponentType))) ComponentType(pType, pKernel, uid, initParams);
      ptr->pFreeFunc = [](void *mem) { epFree(mem); };
      return ptr;
    };
  }
};

template<typename ComponentType, typename Impl>
inline const ComponentDesc* Kernel::RegisterComponentType()
{
  // check the class has a 'super' member
  static_assert(CreateHelper<ComponentType>::HasDescriptor(), "Missing descriptor: needs 'EP_DECLARE_COMPONENT(Name, SuperType, Version, Description)' declared at the top of ComponentType");

  ComponentDesc desc;
  desc.info = ComponentType::MakeDescriptor();
  desc.baseClass = CreateHelper<ComponentType>::GetSuper();

  desc.pInit = CreateHelper<ComponentType>::GetStaticInit();
  desc.pCreateInstance = CreateHelper<ComponentType>::GetCreateFunc();
  desc.pCreateImpl = CreateHelper<ComponentType, Impl>::GetCreateImpl();

  desc.properties = CreateHelper<ComponentType>::GetProperties();
  desc.methods = CreateHelper<ComponentType>::GetMethods();
  desc.events = CreateHelper<ComponentType>::GetEvents();
  desc.staticFuncs = CreateHelper<ComponentType>::GetStaticFuncs();

  desc.pSuperDesc = nullptr;

  if (!desc.info.id.eq("component") && !desc.info.id.eq("icomponent"))
  {
    desc.pSuperDesc = GetComponentDesc(desc.baseClass);
    if (!desc.pSuperDesc)
    {
      EPASSERT(false, "Base class not registered");
      return nullptr;
    }
  }

  return (const ComponentDesc*)RegisterComponentType(desc);
}

template<typename T>
SharedPtr<T> Kernel::CreateComponent(Variant::VarMap initParams)
{
	return shared_pointer_cast<T>(CreateComponent(T::ComponentID(), initParams));
}

// Inlines pipe through C API

inline Kernel* Kernel::GetInstance()
{
  return s_pInstance ? s_pInstance->pKernelInstance : nullptr;
}

template<typename ...Args>
inline void Kernel::LogError(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(1<<0, 2, format);
  else
    Log(1<<0, 2, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogWarning(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(1<<1, level, format);
  else
    Log(1<<1, level, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogDebug(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(1<<2, level, format);
  else
    Log(1<<2, level, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogInfo(int level, String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(1<<3, level, format);
  else
    Log(1<<3, level, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogScript(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(1<<4, 2, format);
  else
    Log(1<<4, 2, MutableString128(Format, format, args...));
}
template<typename ...Args>
inline void Kernel::LogTrace(String format, Args... args) const
{
  if (sizeof...(Args) == 0)
    Log(1<<5, 2, format);
  else
    Log(1<<5, 2,MutableString128(Format, format, args...));
}

} // namespace ep
