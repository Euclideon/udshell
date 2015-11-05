namespace ep {

inline SharedString Component::GetUID() const
{
  return uid;
}
inline SharedString Component::GetName() const
{
  return name;
}

inline bool Component::IsType(String type) const
{
  return s_pPluginInstance->pComponentAPI->IsType((epComponent*)this, type);
}

inline Variant Component::GetProperty(String property) const
{
  return s_pPluginInstance->pComponentAPI->GetProperty((epComponent*)this, property);
}
inline void Component::SetProperty(String property, const Variant &value)
{
  s_pPluginInstance->pComponentAPI->SetProperty((epComponent*)this, property, (const epVariant*)&value);
}

inline Variant Component::CallMethod(String method, Slice<const Variant> args)
{
  return s_pPluginInstance->pComponentAPI->CallMethod((epComponent*)this, method, (const epVariant*)args.ptr, args.length);
}
template<typename ...Args>
inline Variant Component::CallMethod(String method, Args... args)
{
  const Variant varargs[sizeof...(Args)+1] = { args... };
  return CallMethod(method, Slice<const Variant>(varargs, sizeof...(Args)));
}

inline void Component::Subscribe(String eventName, Variant::VarDelegate delegate)
{
  s_pPluginInstance->pComponentAPI->Subscribe((epComponent*)this, eventName, (const epVarDelegate*&)delegate);
}

inline epResult Component::SendMessage(String target, String message, const Variant &data)
{
  return s_pPluginInstance->pComponentAPI->SendMessage((epComponent*)this, target, message, (const epVariant*)&data);
}

} // namespace ep
