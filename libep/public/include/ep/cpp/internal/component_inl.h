namespace ep {

inline Kernel& Component::getKernel() const
{
  return *pKernel;
}

inline const ComponentDesc* Component::getDescriptor() const
{
  return pType;
}

inline SharedString Component::getUid() const
{
  return uid;
}
inline SharedString Component::getName() const
{
  return name;
}
inline void Component::setName(SharedString _name)
{
  pImpl->SetName(_name);
}

inline Variant Component::get(String property) const
{
  return pImpl->Get(property);
}
inline void Component::set(String property, const Variant &value)
{
  pImpl->Set(property, value);
}

inline Variant Component::call(String method, Slice<const Variant> args)
{
  return pImpl->Call(method, args);
}

inline SubscriptionRef Component::subscribe(String eventName, const VarDelegate &delegate)
{
  return pImpl->Subscribe(eventName, delegate);
}
template<typename ...Args>
inline SubscriptionRef Component::subscribe(String eventName, const Delegate<void(Args...)> &d)
{
  typedef SharedPtr<internal::VarDelegateMemento<void(Args...)>> VarDelegateMementoRef;
  return subscribe(eventName, VarDelegate(VarDelegateMementoRef::create(d)));
}

inline VarDelegate Component::getGetterDelegate(String _name, EnumerateFlags enumerateFlags)
{
  const PropertyDesc *pDesc = getPropertyDesc(_name, enumerateFlags);
  if (!pDesc || !pDesc->getter)
    return nullptr;
  return pDesc->getter.getDelegate(this);
}
inline VarDelegate Component::getSetterDelegate(String _name, EnumerateFlags enumerateFlags)
{
  const PropertyDesc *pDesc = getPropertyDesc(_name, enumerateFlags);
  if (!pDesc || !pDesc->setter || pDesc->flags & epPF_Immutable)
    return nullptr;
  return pDesc->setter.getDelegate(this);
}
inline VarDelegate Component::getFunctionDelegate(String _name, EnumerateFlags enumerateFlags)
{
  const MethodDesc *pDesc = getMethodDesc(_name, enumerateFlags);
  if (pDesc)
    return pDesc->method.getDelegate(this);
  // TODO: should also return static functions...
//  const StaticFuncDesc *pStaticFuncs = GetStaticFuncDesc(_name);
//  if (!pStaticFuncs)
//    return pStaticFuncs->staticFunc.getDelegate(this);
  return nullptr;
}

inline Variant Component::save() const
{
  return pImpl->Save();
}

inline void* Component::getUserData() const
{
  return pUserData;
}

inline void Component::addDynamicProperty(const PropertyInfo &property, const MethodShim *pGetter, const MethodShim *pSetter)
{
  pImpl->AddDynamicProperty(property, pGetter, pSetter);
}
inline void Component::addDynamicMethod(const MethodInfo &method, const MethodShim *pMethod)
{
  pImpl->AddDynamicMethod(method, pMethod);
}
inline void Component::addDynamicEvent(const EventInfo &event, const EventShim *pSubscribe)
{
  pImpl->AddDynamicEvent(event, pSubscribe);
}
inline void Component::removeDynamicProperty(String _name)
{
  pImpl->RemoveDynamicProperty(_name);
}
inline void Component::removeDynamicMethod(String _name)
{
  pImpl->RemoveDynamicMethod(_name);
}
inline void Component::removeDynamicEvent(String _name)
{
  pImpl->RemoveDynamicEvent(_name);
}

inline void Component::sendMessage(const ComponentRef &target, String message, const Variant &data) const
{
  return sendMessage(MutableString128(Concat, "@", target->getUid()), message, data);
}

inline void Component::receiveMessage(String message, String sender, const Variant &data)
{
  return pImpl->ReceiveMessage(message, sender, data);
}

template<typename ...Args>
inline Variant Component::call(String method, Args... args)
{
  const Variant varargs[sizeof...(Args)+1] = { args... };
  return call(method, Slice<const Variant>(varargs, sizeof...(Args)));
}

template<typename ...Args>
inline void Component::logError(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<0, 2, text, uid);
  else
    pKernel->Log(1<<0, 2, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::logWarning(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<1, level, text, uid);
  else
    pKernel->Log(1<<1, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::logDebug(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<2, level, text, uid);
  else
    pKernel->Log(1<<2, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::logInfo(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<3, level, text, uid);
  else
    pKernel->Log(1<<3, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::logScript(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<4, 2, text, uid);
  else
    pKernel->Log(1<<4, 2, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::logTrace(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    pKernel->Log(1<<5, 2, text, uid);
  else
    pKernel->Log(1<<5, 2, MutableString128(Format, text, args...), uid);
}

} // namespace ep
