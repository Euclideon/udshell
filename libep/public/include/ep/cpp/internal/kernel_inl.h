#include "ep/cpp/plugin.h"

namespace ep {

namespace internal {
  using ComponentDesc_InitComponentPtr = ComponentDescInl::InitComponent*;
  using ComponentDesc_CreateInstanceCallbackPtr = ComponentDescInl::CreateInstanceCallback*;

  // TODO: HAX HAX! this whole mess needs to be made private somehow...
  template<typename _ComponentType, typename ImplType = void>
  struct CreateHelper
  {
    using CreateFunc = ComponentDescInl::CreateImplCallback*;

    static constexpr bool HasDescriptor() { return HasDescriptorImpl((_ComponentType*)nullptr); }
    static ComponentDescInl::InitComponent* GetStaticInit()
    {
      auto r = GetStaticInitImpl((ImplType*)nullptr);
      if (r)
        return r;
      return GetStaticInitImpl((_ComponentType*)nullptr);
    }

    static ComponentDescInl::CreateInstanceCallback* GetCreateFunc() { return GetCreateFuncImpl((_ComponentType*)nullptr); }

    static SharedString GetSuper() { return GetSuperImpl((typename _ComponentType::Super*)nullptr); }

    static CreateFunc GetCreateImpl() { return GetCreateImplImpl((typename _ComponentType::Impl*)nullptr); }

  private:
    // TODO: these need to go somewhere else, instantiating this class for these functions instantiates the other functions too that should be specialised for IComponent and friends
    template <typename T>
    static constexpr auto HasDescriptorImpl(T* t) -> decltype(T::ComponentInfo(), bool()) { return true; }
    static constexpr bool HasDescriptorImpl(...) { return false; }
    template <typename T>
    static auto GetStaticInitImpl(T* t) -> decltype(T::StaticInit(nullptr), internal::ComponentDesc_InitComponentPtr()) { return &T::StaticInit; }
    static ComponentDescInl::InitComponent* GetStaticInitImpl(...) { return nullptr; }

    template <typename T, typename std::enable_if<!std::is_same<T, void>::value>::type* = nullptr>
    static SharedString GetSuperImpl(T* t) { return T::ComponentID(); }
    static SharedString GetSuperImpl(...) { return nullptr; }

    template <typename T, typename std::enable_if<!std::is_same<T, void>::value>::type* = nullptr>
    static CreateFunc GetCreateImplImpl(T* t) { return [](Component *pInstance, Variant::VarMap initParams) -> void* { return epNew(ImplType, pInstance, initParams); }; }
    static CreateFunc GetCreateImplImpl(...) { return nullptr; }

    template <typename T>
    static auto GetCreateFuncImpl(T* t) -> decltype(T::CreateInstance(), internal::ComponentDesc_CreateInstanceCallbackPtr()) { return &T::CreateInstance; }
    static ComponentDescInl::CreateInstanceCallback* GetCreateFuncImpl(...)
    {
      return [](const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams) -> ComponentRef {
        MutableString128 t(Format, "New: {0} - {1}", _pType->info.identifier, _uid);
        _pKernel->LogDebug(4, t);
        void *pMem = epAlloc(sizeof(_ComponentType));
        EPTHROW_IF_NULL(pMem, Result::AllocFailure, "Memory allocation failed");
        epscope(fail) { if (pMem) epFree(pMem); };
        _ComponentType *ptr = epConstruct(pMem) _ComponentType(_pType, _pKernel, _uid, initParams);
        // NOTE: we need to cast to ensure we play nice with multi inheritance
        ptr->pFreeFunc = [](RefCounted *pMem) { epFree((_ComponentType*)pMem); };
        return SharedPtr<_ComponentType>(ptr);
      };
    }
  };
}

template<typename _ComponentType, typename ImplType, typename GlueType, typename StaticImpl>
inline const ComponentDesc* Kernel::RegisterComponentType()
{
  // check the class has a 'super' member
  static_assert(internal::CreateHelper<_ComponentType>::HasDescriptor(), "Missing descriptor: needs 'EP_DECLARE_COMPONENT(Namespace, Name, SuperType, Version, Description, Flags)' declared at the top of _ComponentType");

  ComponentDescInl *pDesc = epNew(ComponentDescInl);

  pDesc->info = _ComponentType::ComponentInfo();
  pDesc->baseClass = internal::CreateHelper<_ComponentType>::GetSuper();

  pDesc->pInit = internal::CreateHelper<_ComponentType>::GetStaticInit();
  pDesc->pCreateInstance = internal::CreateHelper<_ComponentType>::GetCreateFunc();
  pDesc->pCreateImpl = internal::CreateHelper<_ComponentType, ImplType>::GetCreateImpl();

  // build search trees
  for (auto &p : _ComponentType::GetPropertiesImpl())
    pDesc->propertyTree.insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : _ComponentType::GetMethodsImpl())
    pDesc->methodTree.insert(m.id, { m, m.pMethod });
  for (auto &e : _ComponentType::GetEventsImpl())
    pDesc->eventTree.insert(e.id, { e, e.pSubscribe });
  for (auto &f : _ComponentType::GetStaticFuncsImpl())
    pDesc->staticFuncTree.insert(f.id, { f, (void*)f.pCall });

  // setup the super class and populate from its meta
  pDesc->pSuperDesc = nullptr;
  if (!pDesc->info.identifier.eq("ep.Component"))
  {
    pDesc->pSuperDesc = GetComponentDesc(pDesc->baseClass);
    EPTHROW_IF(!pDesc->pSuperDesc, Result::InvalidType, "Base Component '{0}' not registered", pDesc->baseClass);
    pDesc->PopulateFromDesc((const ep::ComponentDescInl*)pDesc->pSuperDesc);
  }

  RegisterGlueType<GlueType>();
  CreateStaticImpl<StaticImpl, _ComponentType>::Do();

  return RegisterComponentType(pDesc);
}

template<typename GlueType>
inline void Kernel::RegisterGlueType()
{
  pImpl->RegisterGlueType(GlueType::ComponentID(), [](Kernel *_pKernel, const ComponentDesc *_pType, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams) -> ComponentRef {
    void *pMem = epAlloc(sizeof(GlueType));
    EPTHROW_IF_NULL(pMem, Result::AllocFailure, "Memory allocation failed");
    epscope(fail) { if (pMem) epFree(pMem); };
    GlueType *ptr = epConstruct (pMem) GlueType(_pType, _pKernel, _uid, spInstance, initParams);
    ptr->pFreeFunc = [](RefCounted *pMem) { epFree((GlueType*)pMem); };
    return ComponentRef(ptr);
  });
}
template<>
inline void Kernel::RegisterGlueType<void>()
{
}

template<typename StaticImpl, typename ComponentType>
struct Kernel::CreateStaticImpl {
  static inline void Do()
  {
    internal::AddStaticImpl(ComponentType::ComponentID(), UniquePtr<StaticImpl>::create());
  }
};
template<typename T>
struct Kernel::CreateStaticImpl<void, T> { static inline void Do() {} };


template<typename _ComponentType>
Array<const ep::ComponentDesc *> Kernel::GetDerivedComponentDescs(bool bIncludeBase)
{
  return pImpl->GetDerivedComponentDescsFromString(_ComponentType::ComponentID(), bIncludeBase);
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
