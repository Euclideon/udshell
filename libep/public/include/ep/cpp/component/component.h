//! \file component.h
//! \brief Base class for Euclideon Platform Components
//! \author Manu Evans

#pragma once
#if !defined(_EP_COMPONENT_HPP)
#define _EP_COMPONENT_HPP

#include "ep/c/plugin.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/componentdesc.h"
#include "ep/cpp/internal/i/icomponent.h"

namespace ep {

SHARED_CLASS(Component);
SHARED_CLASS(Kernel);

// component API
class Component : public RefCounted
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Component, IComponent, void, EPKERNEL_PLUGINVERSION, "Base component", 0)

public:
  // virtual members
  virtual Variant Save() const;

  virtual Array<SharedString> EnumerateProperties(EnumerateFlags enumerateFlags = 0) const { return pImpl->EnumerateProperties(enumerateFlags); }
  virtual Array<SharedString> EnumerateFunctions(EnumerateFlags enumerateFlags = 0) const { return pImpl->EnumerateFunctions(enumerateFlags); }
  virtual Array<SharedString> EnumerateEvents(EnumerateFlags enumerateFlags = 0) const { return pImpl->EnumerateEvents(enumerateFlags); }

  virtual const PropertyDesc *GetPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const { return pImpl->GetPropertyDesc(_name, enumerateFlags); }
  virtual const MethodDesc *GetMethodDesc(String _name, EnumerateFlags enumerateFlags = 0) const { return pImpl->GetMethodDesc(_name, enumerateFlags); }
  virtual const EventDesc *GetEventDesc(String _name, EnumerateFlags enumerateFlags = 0) const { return pImpl->GetEventDesc(_name, enumerateFlags); }
  virtual const StaticFuncDesc *GetStaticFuncDesc(String _name, EnumerateFlags enumerateFlags = 0) const { return pImpl->GetStaticFuncDesc(_name, enumerateFlags); }


  // regular members
  const SharedString uid;

  //! Gets the components kernel instance.
  //! \return Returns the components kernel instance.
  //! \see GetDescriptor, GetType, GetDisplayName, GetDescription, GetUid, GetName
  Kernel& GetKernel() const;

  //! Gets the components descriptor
  //! \return Returns the components descriptor
  //! \see GetKernel, GetType, GetDisplayName, GetDescription, GetUid, GetName
  const ComponentDesc* GetDescriptor() const;

  //! Gets the components type identifier.
  //! \return Returns the component type as a string.
  //! \see IsType, GetDescriptor, GetKernel, GetDisplayName, GetDescription, GetUid, GetName
  String GetType() const { return pType->info.identifier; }

  //! Gets a user-friendly name for the component.
  //! \return Returns the component display name.
  //! \see GetDescriptor, GetKernel, GetType, GetDescription, GetUid, GetName
  String GetDisplayName() const { return pType->info.displayName; }

  //! Gets a user-friendly description of the component.
  //! \return Returns the component description.
  //! \see GetDescriptor, GetKernel, GetType, GetDisplayName, GetUid, GetName
  String GetDescription() const { return pType->info.description; }

  //! Gets the components unique identifier.
  //! \return Returns the component uid.
  //! \see GetDescriptor, GetKernel, GetType, GetDisplayName, GetDescription, GetName
  SharedString GetUid() const;

  //! Gets a user-specified name for the component.
  //! \return Returns the component name, or an empty string if no name has been assigned.
  //! \see SetName, GetDescriptor, GetKernel, GetType, GetDisplayName, GetDescription, GetUid
  SharedString GetName() const;

  //! Sets a user-specified name for the component.
  //! \param name Name to be assigned to the component.
  //! \return None.
  //! \see GetName
  void SetName(SharedString name);

  //! Checks to see if \a T is present in the components inheritance hierarchy.
  //! \tparam T Type to check.
  //! \return Returns \c true if the component has the specified type in its inheritance hierarchy.
  //! \see GetType
  template<typename T>
  bool IsType() const { return IsType(T::ComponentID()); }

  //! Checks to see if the component has the named type in its inheritance hierarchy.
  //! \param type Type name to check.
  //! \return Returns \c true if the component has the specified type in its inheritance hierarchy.
  //! \see GetType
  bool IsType(String type) const;


  // meta access

  //! Gets a delegate for the specified property's getter method.
  //! \param name Name of the property.
  //! \param enumerateFlags Flags that control property enumeration.
  //! \return Returns a Delegate for the property's getter.
  //! \see GetSetterDelegate, GetFunctionDelegate
  VarDelegate GetGetterDelegate(String name, EnumerateFlags enumerateFlags = 0);

  //! Gets a delegate for the specified property's setter method.
  //! \param name Name of the property.
  //! \param enumerateFlags Flags that control property enumeration.
  //! \return Returns a Delegate for the property's setter.
  //! \see GetGetterDelegate, GetFunctionDelegate
  VarDelegate GetSetterDelegate(String name, EnumerateFlags enumerateFlags = 0);

  //! Gets a delegate for the specified function or method.
  //! \param name Name of the function or method.
  //! \param enumerateFlags Flags that control property enumeration.
  //! \return Returns a Delegate for the function.
  //! \see GetGetterDelegate, GetSetterDelegate
  VarDelegate GetFunctionDelegate(String name, EnumerateFlags enumerateFlags = 0);


  // meta interface

  //! Gets the value of a components property.
  //! \param property Name of the property.
  //! \return The current value of the specified property.
  //! \see Set, Call
  Variant Get(String property) const;

  //! Sets the value of a components property.
  //! \param property Name of the property.
  //! \param value Value to assign to the property.
  //! \return None.
  //! \see Get, Call
  void Set(String property, const Variant &value);

  //! Calls a components method or function.
  //! \param method Name of method or function.
  //! \param args Slice of args to pass to the function.
  //! \return Variant containing the function's return value, or \c void if the function has no return value.
  //! \see Get, Set
  Variant Call(String method, Slice<const Variant> args);

  //! Calls a components method or function.
  //! \param method Name of method or function.
  //! \param args Variadic list of arguments to pass to the function.
  //! \return Variant containing the function's return value, or \c void if the function has no return value.
  //! \remarks \a args must each be convertible to Variant, otherwise \c Call will throw.
  //! \see Get, Set
  template<typename ...Args>
  Variant Call(String method, Args... args);

  SubscriptionRef Subscribe(String eventName, const VarDelegate &delegate);
  template<typename ...Args>
  SubscriptionRef Subscribe(String eventName, const Delegate<void(Args...)> &d);
  template <class X, class Y, typename ...Args>
  SubscriptionRef Subscribe(String eventName, Y *pThis, void(X::*pMethod)(Args...)) { return Subscribe(eventName, Delegate<void(Args...)>(pThis, pMethod)); }
  template <class X, class Y, typename ...Args>
  SubscriptionRef Subscribe(String eventName, Y *pThis, void(X::*pMethod)(Args...) const) { return Subscribe(eventName, Delegate<void(Args...)>(pThis, pMethod)); }
  template <typename ...Args>
  SubscriptionRef Subscribe(String eventName, void(*pFunc)(Args...)) { return Subscribe(eventName, Delegate<void(Args...)>(pFunc)); }

  void SendMessage(String target, String message, const Variant &data) const;
  void SendMessage(const ComponentRef &target, String message, const Variant &data) const;

  template<typename ...Args> void LogError(String text, Args... args) const;
  template<typename ...Args> void LogWarning(int level, String text, Args... args) const;
  template<typename ...Args> void LogDebug(int level, String text, Args... args) const;
  template<typename ...Args> void LogInfo(int level, String text, Args... args) const;
  template<typename ...Args> void LogScript(String text, Args... args) const;
  template<typename ...Args> void LogTrace(String text, Args... args) const;

  void* GetUserData() const;

  void AddDynamicProperty(const PropertyInfo &property, const MethodShim *pGetter = nullptr, const MethodShim *pSetter = nullptr);
  void AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod = nullptr);
  void AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe = nullptr);
  void RemoveDynamicProperty(String name);
  void RemoveDynamicMethod(String name);
  void RemoveDynamicEvent(String name);

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

  virtual void ReceiveMessage(String message, String sender, const Variant &data);

  Component(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams);
  ~Component()
  {
    pImpl = nullptr;
    // unregistered component types aren't in the registry and will be deleted when the component destroys
    while (pType && pType->info.flags & ComponentInfoFlags::Unregistered)
    {
      const ComponentDesc *pDesc = pType;
      (const ComponentDesc*&)pType = pType->pSuperDesc;
      epDelete(pDesc);
    }
  }

  Component(const Component &) = delete;    // Still not sold on this
  void operator=(const Component &) = delete;

  SharedString name;

  const ComponentDesc *const pType;
  class Kernel *const pKernel;

  void *pUserData = nullptr;

  void* CreateImplInternal(String ComponentType, Variant::VarMap initParams);

private:
  Array<const PropertyInfo> GetProperties() const;
  Array<const MethodInfo> GetMethods() const;
};

// component cast
template<typename T>
inline SharedPtr<T> component_cast(const ComponentRef &spComponent)
{
  EPTHROW_IF(!spComponent, Result::BadCast, "component is null");

  const ComponentDesc *pDesc = spComponent->GetDescriptor();
  while (pDesc)
  {
    if (pDesc->info.identifier.eq(T::ComponentID()))
      return shared_pointer_cast<T>(spComponent);
    pDesc = pDesc->pSuperDesc;
  }
  EPTHROW(Result::BadCast, "component cast failed");
}
// TODO: cast for IComponent types...

ptrdiff_t epStringify(Slice<char> buffer, String format, const Component *pComponent, const VarArg *pArgs);

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
