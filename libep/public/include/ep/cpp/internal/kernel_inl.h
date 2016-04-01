#include "ep/cpp/plugin.h"

namespace ep {

namespace internal {
  using ComponentDesc_InitComponentPtr = ComponentDescInl::InitComponent*;
  using ComponentDesc_CreateInstanceCallbackPtr = ComponentDescInl::CreateInstanceCallback*;
}

// TODO: HAX HAX! this whole mess needs to be made private somehow...
template<typename _ComponentType, typename ImplType>
struct Kernel::CreateHelper
{
  using CreateFunc = ComponentDescInl::CreateImplCallback*;

  static constexpr bool HasDescriptor() { return HasDescriptorImpl((_ComponentType*)nullptr); }
  static Array<const PropertyInfo> GetProperties() { return GetPropertiesImpl((_ComponentType*)nullptr); }
  static Array<const MethodInfo> GetMethods() { return GetMethodsImpl((_ComponentType*)nullptr); }
  static Array<const EventInfo> GetEvents() { return GetEventsImpl((_ComponentType*)nullptr); }
  static Array<const StaticFuncInfo> GetStaticFuncs() { return GetStaticFuncsImpl((_ComponentType*)nullptr); }
  static ComponentDescInl::InitComponent* GetStaticInit() { return GetStaticInitImpl((_ComponentType*)nullptr); }

  static ComponentDescInl::CreateInstanceCallback* GetCreateFunc() { return GetCreateFuncImpl((_ComponentType*)nullptr); }

  static SharedString GetSuper() { return GetSuperImpl((typename _ComponentType::Super*)nullptr); }

  static CreateFunc GetCreateImpl() { return GetCreateImplImpl((typename _ComponentType::Impl*)nullptr); }

private:
  template <typename Owner, typename C, typename R, typename... Args>
  static constexpr bool FunctionBelongsTo(R(C::*)(Args...) const) { return std::is_same<C, Owner>::value; }

  // TODO: these need to go somewhere else, instantiating this class for these functions instantiates the other functions too that should be specialised for IComponent and friends
  template <typename T>
  static constexpr auto HasDescriptorImpl(T* t) -> decltype(T::ComponentInfo(), bool()) { return true; }
  static constexpr bool HasDescriptorImpl(...) { return false; }
  template <typename T>
  static auto GetPropertiesImpl(const T* t) -> decltype(std::enable_if<FunctionBelongsTo<T>(&T::GetProperties)>::type(), Array<const PropertyInfo>()) { return t->GetProperties(); }
  static Array<const PropertyInfo> GetPropertiesImpl(...) { return nullptr; }
  template <typename T>
  static auto GetMethodsImpl(const T* t) -> decltype(std::enable_if<FunctionBelongsTo<T>(&T::GetMethods)>::type(), Array<const MethodInfo>()) { return t->GetMethods(); }
  static Array<const MethodInfo> GetMethodsImpl(...) { return nullptr; }
  template <typename T>
  static auto GetEventsImpl(const T* t) -> decltype(std::enable_if<FunctionBelongsTo<T>(&T::GetEvents)>::type(), Array<const EventInfo>()) { return t->GetEvents(); }
  static Array<const EventInfo> GetEventsImpl(...) { return nullptr; }
  template <typename T>
  static auto GetStaticFuncsImpl(const T* t) -> decltype(std::enable_if<FunctionBelongsTo<T>(&T::GetStaticFuncs)>::type(), Array<const StaticFuncInfo>()) { return t->GetStaticFuncs(); }
  static Array<const StaticFuncInfo> GetStaticFuncsImpl(...) { return nullptr; }
  template <typename T>
  static auto GetStaticInitImpl(T* t) -> decltype(T::StaticInit(nullptr), internal::ComponentDesc_InitComponentPtr()) { return &T::StaticInit; }
  static ComponentDescInl::InitComponent* GetStaticInitImpl(...) { return nullptr; }

  template <typename T, typename std::enable_if<!std::is_same<T, void>::value>::type* = nullptr>
  static SharedString GetSuperImpl(T* t) { return T::ComponentID(); }
  static SharedString GetSuperImpl(...) { return nullptr; }

  template <typename T, typename std::enable_if<!std::is_same<T, void>::value>::type* = nullptr>
  static CreateFunc GetCreateImplImpl(T* t) { return [](Component *pInstance, Variant::VarMap initParams) -> void* { return epNew ImplType(pInstance, initParams); }; }
  static CreateFunc GetCreateImplImpl(...) { return nullptr; }

  template <typename T>
  static auto GetCreateFuncImpl(T* t) -> decltype(T::CreateInstance(), internal::ComponentDesc_CreateInstanceCallbackPtr()) { return &T::CreateInstance; }
  static ComponentDescInl::CreateInstanceCallback* GetCreateFuncImpl(...)
  {
    return [](const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams) -> ComponentRef {
      MutableString128 t(Format, "New: {0} - {1}", _pType->info.identifier, _uid);
      _pKernel->LogDebug(4, t);
      // TODO: this new can't exist in the wild... need to call back into kernel!!
      void *pMem = epAlloc(sizeof(_ComponentType));
      epscope(fail) { if (pMem) epFree(pMem); };
      EPTHROW_IF_NULL(pMem, epR_AllocFailure, "Memory allocation failed");
      _ComponentType *ptr = new (pMem) _ComponentType(_pType, _pKernel, _uid, initParams);
      // NOTE: we need to cast to ensure we play nice with multi inheritance
      ptr->pFreeFunc = [](RefCounted *pMem) { epFree((_ComponentType*)pMem); };
      return SharedPtr<_ComponentType>(ptr);
    };
  }
};

template<typename _ComponentType, typename ImplType, typename GlueType>
inline const ComponentDesc* Kernel::RegisterComponentType()
{
  // check the class has a 'super' member
  static_assert(CreateHelper<_ComponentType>::HasDescriptor(), "Missing descriptor: needs 'EP_DECLARE_COMPONENT(Namespace, Name, SuperType, Version, Description)' declared at the top of _ComponentType");

  ComponentDescInl *pDesc = epNew ComponentDescInl;

  pDesc->info = _ComponentType::ComponentInfo();
  pDesc->baseClass = CreateHelper<_ComponentType>::GetSuper();

  pDesc->pInit = CreateHelper<_ComponentType>::GetStaticInit();
  pDesc->pCreateInstance = CreateHelper<_ComponentType>::GetCreateFunc();
  pDesc->pCreateImpl = CreateHelper<_ComponentType, ImplType>::GetCreateImpl();

  // build search trees
  for (auto &p : CreateHelper<_ComponentType>::GetProperties())
    pDesc->propertyTree.Insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : CreateHelper<_ComponentType>::GetMethods())
    pDesc->methodTree.Insert(m.id, { m, m.pMethod });
  for (auto &e : CreateHelper<_ComponentType>::GetEvents())
    pDesc->eventTree.Insert(e.id, { e, e.pSubscribe });
  for (auto &f : CreateHelper<_ComponentType>::GetStaticFuncs())
    pDesc->staticFuncTree.Insert(f.id, { f, (void*)f.pCall });

  // setup the super class and populate from its meta
  pDesc->pSuperDesc = nullptr;
  if (!pDesc->info.identifier.eq("ep.component"))
  {
    pDesc->pSuperDesc = GetComponentDesc(pDesc->baseClass);
    EPTHROW_IF(!pDesc->pSuperDesc, epR_InvalidType, "Base Component '{0}' not registered", pDesc->baseClass);
    pDesc->PopulateFromDesc((const ep::ComponentDescInl*)pDesc->pSuperDesc);
  }

  RegisterGlueType<GlueType>();

  return RegisterComponentType(pDesc);
}

template<typename GlueType>
inline void Kernel::RegisterGlueType()
{
  pImpl->RegisterGlueType(GlueType::ComponentID(), [](Kernel *_pKernel, const ComponentDesc *_pType, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams) -> ComponentRef {
    // TODO: this new can't exist in the wild... need to call back into kernel!!
    void *pMem = epAlloc(sizeof(GlueType));
    epscope(fail) { if (pMem) epFree(pMem); };
    EPTHROW_IF_NULL(pMem, epR_AllocFailure, "Memory allocation failed");
    GlueType *ptr = new (pMem) GlueType(_pType, _pKernel, _uid, spInstance, initParams);
    ptr->pFreeFunc = [](RefCounted *pMem) { epFree((GlueType*)pMem); };
    return ComponentRef(ptr);
  });
}
template<>
inline void Kernel::RegisterGlueType<void>()
{
}


template<typename _ComponentType>
Array<const ep::ComponentDesc *> Kernel::GetDerivedComponentDescs(bool bIncludeBase)
{
  return pImpl->GetDerivedComponentDescs(_ComponentType::ComponentID(), bIncludeBase);
}

template<typename T>
SharedPtr<T> Kernel::CreateComponent(Variant::VarMap initParams)
{
	return shared_pointer_cast<T>(CreateComponent(T::ComponentID(), initParams));
}

template<typename T>
SharedPtr<T> Kernel::CreateGlue(const ComponentDesc *_pType, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams)
{
  return shared_pointer_cast<T>(CreateGlue(T::ComponentID(), _pType, _uid, spInstance, initParams));
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
