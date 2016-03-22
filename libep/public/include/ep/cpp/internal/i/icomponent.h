#pragma once
#if !defined(_EP_ICOMPONENT_HPP)
#define _EP_ICOMPONENT_HPP

#include "ep/cpp/variant.h"

namespace ep {

EP_BITFIELD(EnumerateFlags,
  NoStatic,
  NoDynamic
);

class IComponent
{
  friend class Component;
public:
  virtual void SetName(SharedString name) = 0;

  virtual Array<SharedString> EnumerateProperties(EnumerateFlags enumerateFlags) const = 0;
  virtual Array<SharedString> EnumerateFunctions(EnumerateFlags enumerateFlags) const = 0;
  virtual Array<SharedString> EnumerateEvents(EnumerateFlags enumerateFlags) const = 0;

  virtual const PropertyDesc *GetPropertyDesc(String name, EnumerateFlags enumerateFlags) const = 0;
  virtual const MethodDesc *GetMethodDesc(String name, EnumerateFlags enumerateFlags) const = 0;
  virtual const EventDesc *GetEventDesc(String name, EnumerateFlags enumerateFlags) const = 0;
  virtual const StaticFuncDesc *GetStaticFuncDesc(String name, EnumerateFlags enumerateFlags) const = 0;

  virtual Variant Get(String property) const = 0;
  virtual void Set(String property, const Variant &value) = 0;
  virtual Variant Call(String function, Slice<const Variant> args) = 0;
  virtual void Subscribe(String eventName, const VarDelegate &delegate) = 0;

  virtual Variant Save() const = 0;

  virtual void AddDynamicProperty(const PropertyInfo &property, const MethodShim *pGetter = nullptr, const MethodShim *pSetter = nullptr) = 0;
  virtual void AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod = nullptr) = 0;
  virtual void AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe = nullptr) = 0;
  virtual void RemoveDynamicProperty(String name) = 0;
  virtual void RemoveDynamicMethod(String name) = 0;
  virtual void RemoveDynamicEvent(String name) = 0;

protected:
  virtual void ReceiveMessage(String message, String sender, const Variant &data) = 0;

private:
  virtual void Init(Variant::VarMap initParams) = 0;
};

} // namespace ep

#endif // _EP_ICOMPONENT_HPP
