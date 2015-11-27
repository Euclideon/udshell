namespace ep {

inline Kernel& Component::GetKernel() const
{
  return *(Kernel*)pKernel;
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
  name = _name;
}

inline void* Component::GetUserData() const
{
  return pUserData;
}

inline epResult Component::SendMessage(Component *pComponent, String message, const Variant &data) const
{
  return SendMessage(MutableString128(Concat, "@", pComponent->uid), message, data);
}

template<typename ...Args>
inline Variant Component::CallMethod(String method, Args... args)
{
  const Variant varargs[sizeof...(Args)+1] = { args... };
  return CallMethod(method, Slice<const Variant>(varargs, sizeof...(Args)));
}

template<typename ...Args>
inline void Component::LogError(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(0, 2, text, uid);
  else
    LogInternal(0, 2, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogWarning(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(1, level, text, uid);
  else
    LogInternal(1, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogDebug(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(2, level, text, uid);
  else
    LogInternal(2, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogInfo(int level, String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(3, level, text, uid);
  else
    LogInternal(3, level, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogScript(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(4, 2, text, uid);
  else
    LogInternal(4, 2, MutableString128(Format, text, args...), uid);
}
template<typename ...Args>
inline void Component::LogTrace(String text, Args... args) const
{
  if (sizeof...(Args) == 0)
    LogInternal(5, 2, text, uid);
  else
    LogInternal(5, 2, MutableString128(Format, text, args...), uid);
}

} // namespace ep
