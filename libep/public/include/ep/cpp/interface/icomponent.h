#pragma once
#if !defined(_EP_ICOMPONENT_HPP)
#define _EP_ICOMPONENT_HPP

#include "ep/cpp/variant.h"

namespace ep {

class IComponent
{
  friend class Component;
public:
  virtual void SetName(SharedString name) = 0;
  virtual Variant GetProperty(String property) const = 0;
  virtual void SetProperty(String property, const Variant &value) = 0;
  virtual Variant CallMethod(String method, Slice<const Variant> args) = 0;
  virtual void Subscribe(String eventName, const Variant::VarDelegate &delegate) = 0;
  virtual Variant Save() const = 0;
private:
  virtual void Init(Variant::VarMap initParams) = 0;
  virtual epResult InitComplete() = 0;
  virtual epResult ReceiveMessage(String message, String sender, const Variant &data) = 0;
  virtual void AddDynamicProperty(const PropertyInfo &property) = 0;
  virtual void AddDynamicMethod(const MethodInfo &method) = 0;
  virtual void AddDynamicEvent(const EventInfo &event) = 0;
  virtual void RemoveDynamicProperty(String name) = 0;
  virtual void RemoveDynamicMethod(String name) = 0;
  virtual void RemoveDynamicEvent(String name) = 0;
};

} // namespace ep

#endif // _EP_ICOMPONENT_HPP
