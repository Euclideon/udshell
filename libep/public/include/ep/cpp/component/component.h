#pragma once
#if !defined(_EP_COMPONENT_HPP)
#define _EP_COMPONENT_HPP

#include "ep/c/plugin.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/componentdesc.h"
#include "ep/cpp/internal/i/icomponent.h"
#include "ep/c/internal/component_inl.h"

namespace ep {

SHARED_CLASS(Component);
SHARED_CLASS(Kernel);

// component API
class Component : public RefCounted, public IComponent
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Component, IComponent, void, EPKERNEL_PLUGINVERSION, "Base component")
public:

  const SharedString uid;

  Kernel& GetKernel() const;
  const ComponentDesc* GetDescriptor() const;

  String GetType() const { return pType->info.id; }
  String GetDisplayName() const { return pType->info.displayName; }
  String GetDescription() const { return pType->info.description; }

  SharedString GetUid() const;
  SharedString GetName() const;
  void SetName(SharedString _name) override final;

  template<typename T>
  bool IsType() const { return IsType(T::ComponentID()); }
  bool IsType(String type) const;

  // meta access
  Array<SharedString> EnumerateProperties(EnumerateFlags enumerateFlags = 0) const override { return pImpl->EnumerateProperties(enumerateFlags); }
  Array<SharedString> EnumerateFunctions(EnumerateFlags enumerateFlags = 0) const override { return pImpl->EnumerateFunctions(enumerateFlags); }
  Array<SharedString> EnumerateEvents(EnumerateFlags enumerateFlags = 0) const override { return pImpl->EnumerateEvents(enumerateFlags); }

  const PropertyDesc *GetPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pImpl->GetPropertyDesc(_name, enumerateFlags); }
  const MethodDesc *GetMethodDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pImpl->GetMethodDesc(_name, enumerateFlags); }
  const EventDesc *GetEventDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pImpl->GetEventDesc(_name, enumerateFlags); }
  const StaticFuncDesc *GetStaticFuncDesc(String _name, EnumerateFlags enumerateFlags = 0) const override { return pImpl->GetStaticFuncDesc(_name, enumerateFlags); }

  VarDelegate GetGetterDelegate(String name, EnumerateFlags enumerateFlags = 0);
  VarDelegate GetSetterDelegate(String name, EnumerateFlags enumerateFlags = 0);
  VarDelegate GetFunctionDelegate(String name, EnumerateFlags enumerateFlags = 0);

  // meta interface
  Variant Get(String property) const override final;
  void Set(String property, const Variant &value) override final;

  Variant Call(String method, Slice<const Variant> args) override final;
  template<typename ...Args>
  Variant Call(String method, Args... args);

  void Subscribe(String eventName, const VarDelegate &delegate) override final;
  template<typename ...Args>
  void Subscribe(String eventName, const Delegate<void(Args...)> &d);
  template <class X, class Y, typename ...Args>
  void Subscribe(String eventName, Y *pThis, void(X::*pMethod)(Args...)) { Subscribe(eventName, Delegate<void(Args...)>(pThis, pMethod)); }
  template <class X, class Y, typename ...Args>
  void Subscribe(String eventName, Y *pThis, void(X::*pMethod)(Args...) const) { Subscribe(eventName, Delegate<void(Args...)>(pThis, pMethod)); }
  template <typename ...Args>
  void Subscribe(String eventName, void(*pFunc)(Args...)) { Subscribe(eventName, Delegate<void(Args...)>(pFunc)); }

  void SendMessage(String target, String message, const Variant &data) const;
  void SendMessage(const ComponentRef &target, String message, const Variant &data) const;

  Variant Save() const override;

  template<typename ...Args> void LogError(String text, Args... args) const;
  template<typename ...Args> void LogWarning(int level, String text, Args... args) const;
  template<typename ...Args> void LogDebug(int level, String text, Args... args) const;
  template<typename ...Args> void LogInfo(int level, String text, Args... args) const;
  template<typename ...Args> void LogScript(String text, Args... args) const;
  template<typename ...Args> void LogTrace(String text, Args... args) const;

  void* GetUserData() const;

  void AddDynamicProperty(const PropertyInfo &property, const MethodShim *pGetter = nullptr, const MethodShim *pSetter = nullptr) override final;
  void AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod = nullptr) override final;
  void AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe = nullptr) override final;
  void RemoveDynamicProperty(String name) override final;
  void RemoveDynamicMethod(String name) override final;
  void RemoveDynamicEvent(String name) override final;

/*
  const PropertyInfo *GetPropertyInfo(String propName) const
  {
    const PropertyDesc *pDesc = GetPropertyDesc(propName);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetMethodInfo(String methName) const
  {
    const MethodDesc *pDesc = GetMethodDesc(methName);
    return pDesc ? &pDesc->info : nullptr;
  }
  const EventInfo *GetEventInfo(String eventName) const
  {
    const EventDesc *pDesc = GetEventDesc(eventName);
    return pDesc ? &pDesc->info : nullptr;
  }
  const FunctionInfo *GetStaticFuncInfo(String staticFuncName) const
  {
    const StaticFuncDesc *pDesc = GetStaticFuncDesc(staticFuncName);
    return pDesc ? &pDesc->info : nullptr;
  }
*/


protected:
  friend class LuaState;
  friend class KernelImpl;

  Component(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams);
  ~Component()
  {
    pImpl = nullptr;
  }

  Component(const Component &) = delete;    // Still not sold on this
  void operator=(const Component &) = delete;

  SharedString name;

  const ComponentDesc *const pType;
  class Kernel *const pKernel;

  void *pUserData = nullptr;

  void* CreateImplInternal(String ComponentType, Variant::VarMap initParams);

  void ReceiveMessage(String message, String sender, const Variant &data) override;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY_RO(Uid, "Component UID", nullptr, 0),
      EP_MAKE_PROPERTY(Name, "Component Name", nullptr, 0),
      EP_MAKE_PROPERTY_RO(Type, "Component Type", nullptr, 0),
      EP_MAKE_PROPERTY_RO(DisplayName, "Component Display Name", nullptr, 0),
      EP_MAKE_PROPERTY_RO(Description, "Component Description", nullptr, 0),
    };
  }

private:
  void Init(Variant::VarMap initParams) override final;
};

// component cast
template<typename T>
inline SharedPtr<T> component_cast(ComponentRef spComponent)
{
  EPTHROW_IF(!spComponent, epR_BadCast, "component is null");

  const ComponentDesc *pDesc = spComponent->GetDescriptor();
  while (pDesc)
  {
    if (pDesc->info.id.eq(T::ComponentID()))
      return shared_pointer_cast<T>(spComponent);
    pDesc = pDesc->pSuperDesc;
  }
  EPTHROW(epR_BadCast, "component cast failed");
}
// TODO: cast for IComponent types...

ptrdiff_t epStringify(Slice<char> buffer, String format, const Component *pComponent, const epVarArg *pArgs);

inline Variant epToVariant(ep::Component *pC)
{
  // Variant does not inc the refcount, effectively a borrow.
  return Variant((const SharedPtr<RefCounted>&)pC, Variant::SharedPtrType::Component, false);
}

// HACK: this is here because forward referencing!
// SharedPtr<Component> (and derived types)
template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr> // O_O
inline void epFromVariant(const Variant &v, SharedPtr<T> *pR)
{
  *pR = component_cast<T>(v.asComponent());
}

} // namespace ep

#include "ep/cpp/kernel.h"
#include "ep/cpp/internal/component_inl.h"

#endif
