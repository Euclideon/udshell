namespace ep {

inline Kernel& Component::GetKernel() const
{
  return *pKernel;
}

inline const ComponentDesc* Component::GetDescriptor() const
{
  return pType;
}

inline SharedString Component::GetUid() const
{
  return uid;
}
inline SharedString Component::GetName() const
{
  return name;
}
inline void Component::SetName(SharedString _name)
{
  pImpl->SetName(_name);
}

inline Variant Component::Get(String property) const
{
  return pImpl->Get(property);
}
inline void Component::Set(String property, const Variant &value)
{
  pImpl->Set(property, value);
}

inline Variant Component::Call(String method, Slice<const Variant> args)
{
  return pImpl->Call(method, args);
}

inline void Component::Subscribe(String eventName, const Variant::VarDelegate &delegate)
{
  pImpl->Subscribe(eventName, delegate);
}
template<typename ...Args>
inline void Component::Subscribe(String eventName, const Delegate<void(Args...)> &d)
{
  typedef SharedPtr<internal::VarDelegateMemento<void(Args...)>> VarDelegateMementoRef;
  Subscribe(eventName, Variant::VarDelegate(VarDelegateMementoRef::create(d)));
}

inline GetterShim::DelegateType Component::GetGetterDelegate(String _name, EnumerateFlags enumerateFlags) const
{
  const PropertyDesc *pDesc = GetPropertyDesc(_name, enumerateFlags);
  if (!pDesc || !pDesc->getter)
  {
    // TODO: throw in this case?
    const char *pMessage = pDesc ? "Property '{0}' for component '{1}' is write-only" : "No property '{0}' for component '{1}'";
    LogWarning(2, pMessage, _name, name.empty() ? uid : name);
    return nullptr;
  }
  return pDesc->getter.getDelegate(this);
}
inline SetterShim::DelegateType Component::GetSetterDelegate(String _name, EnumerateFlags enumerateFlags)
{
  const PropertyDesc *pDesc = GetPropertyDesc(_name, enumerateFlags);
  if (!pDesc || !pDesc->setter || pDesc->flags & epPF_Immutable)
  {
    // TODO: throw in this case?
    const char *pMessage = pDesc ? "Property '{0}' for component '{1}' is read-only" : "No property '{0}' for component '{1}'";
    LogWarning(2, pMessage, _name, name.empty() ? uid : name);
    return nullptr;
  }
  return pDesc->setter.getDelegate(this);
}
inline MethodShim::DelegateType Component::GetFunctionDelegate(String _name, EnumerateFlags enumerateFlags)
{
  const MethodDesc *pDesc = GetMethodDesc(_name, enumerateFlags);
  if (pDesc)
    return pDesc->method.getDelegate(this);

  // TODO: should also return static functions...
//  const StaticFuncDesc *pStaticFuncs = GetStaticFuncDesc(_name);
//  if (!pStaticFuncs)
//    return pStaticFuncs->staticFunc.getDelegate(this);

  LogWarning(1, "No function '{0}' for component '{1}'", _name, name.empty() ? uid : name);
  return nullptr;
}

inline Variant Component::Save() const
{
  return pImpl->Save();
}

inline void* Component::GetUserData() const
{
  return pUserData;
}

inline void Component::AddDynamicProperty(const PropertyInfo &property, const GetterShim *pGetter, const SetterShim *pSetter)
{
  pImpl->AddDynamicProperty(property, pGetter, pSetter);
}
inline void Component::AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod)
{
  pImpl->AddDynamicMethod(method, pMethod);
}
inline void Component::AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe)
{
  pImpl->AddDynamicEvent(event, pSubscribe);
}
inline void Component::RemoveDynamicProperty(String _name)
{
  pImpl->RemoveDynamicProperty(_name);
}
inline void Component::RemoveDynamicMethod(String _name)
{
  pImpl->RemoveDynamicMethod(_name);
}
inline void Component::RemoveDynamicEvent(String _name)
{
  pImpl->RemoveDynamicEvent(_name);
}

inline void Component::SendMessage(const ComponentRef &target, String message, const Variant &data) const
{
  return SendMessage(MutableString128(Concat, "@", target->GetUid()), message, data);
}

inline void Component::Init(Variant::VarMap initParams)
{
  pImpl->Init(initParams);
}

inline void Component::InitComplete()
{
  pImpl->InitComplete();
}

inline void Component::ReceiveMessage(String message, String sender, const Variant &data)
{
  return pImpl->ReceiveMessage(message, sender, data);
}

template<typename ...Args>
inline Variant Component::Call(String method, Args... args)
{
  const Variant varargs[sizeof...(Args)+1] = { args... };
  return Call(method, Slice<const Variant>(varargs, sizeof...(Args)));
}

template<typename ...Args>
inline void Component::LogError(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<0, 2, text, uid);
  else
    pKernel->Log(1<<0, 2, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogWarning(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<1, level, text, uid);
  else
    pKernel->Log(1<<1, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogDebug(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<2, level, text, uid);
  else
    pKernel->Log(1<<2, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogInfo(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<3, level, text, uid);
  else
    pKernel->Log(1<<3, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogScript(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<4, 2, text, uid);
  else
    pKernel->Log(1<<4, 2, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogTrace(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<5, 2, text, uid);
  else
    pKernel->Log(1<<5, 2, MutableString128(Format, text, args...), uid);
}

} // namespace ep
