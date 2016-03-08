#pragma once
#ifndef EPCOMPONENT_H
#define EPCOMPONENT_H

#include "ep/cpp/componentdesc.h"
#include "ep/cpp/event.h"
#include "ep/cpp/internal/i/icomponent.h"
#include "ep/cpp/component/component.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif

namespace ep {

class ComponentImpl : public BaseImpl<Component, IComponent>
{
public:
  ComponentImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance) {}

  void SetName(SharedString name) override final;

  Array<SharedString> EnumerateProperties(EnumerateFlags enumerateFlags) const override final;
  Array<SharedString> EnumerateFunctions(EnumerateFlags enumerateFlags) const override final;
  Array<SharedString> EnumerateEvents(EnumerateFlags enumerateFlags) const override final;

  const PropertyDesc *GetPropertyDesc(String name, EnumerateFlags enumerateFlags = 0) const override final;
  const MethodDesc *GetMethodDesc(String name, EnumerateFlags enumerateFlags = 0) const override final;
  const EventDesc *GetEventDesc(String name, EnumerateFlags enumerateFlags = 0) const override final;
  const StaticFuncDesc *GetStaticFuncDesc(String name, EnumerateFlags enumerateFlags = 0) const override final;

  Variant Get(String property) const override final;
  void Set(String property, const Variant &value) override final;
  Variant Call(String method, Slice<const Variant> args) override final;
  void Subscribe(String eventName, const Variant::VarDelegate &delegate) override final;

  Variant Save() const override final { return Variant(Variant::VarMap()); }

  void AddDynamicProperty(const PropertyInfo &property, const GetterShim *pGetter, const SetterShim *pSetter) override final;
  void AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod) override final;
  void AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe) override final;
  void RemoveDynamicProperty(String name) override final;
  void RemoveDynamicMethod(String name) override final;
  void RemoveDynamicEvent(String name) override final;

protected:
  ~ComponentImpl();

  void Init(Variant::VarMap initParams) override final;
  void InitComplete() override final {}

  void ReceiveMessage(String message, String sender, const Variant &data) override final;

  // property access
  size_t NumProperties() const { return instanceProperties.Size() + ((const ComponentDescInl*)GetDescriptor())->propertyTree.Size(); }
  size_t NumMethods() const { return instanceMethods.Size() + ((const ComponentDescInl*)GetDescriptor())->methodTree.Size(); }
  size_t NumEvents() const { return instanceEvents.Size() + ((const ComponentDescInl*)GetDescriptor())->eventTree.Size(); }
  size_t NumStaticFuncs() const { return ((const ComponentDescInl*)GetDescriptor())->staticFuncTree.Size(); }

  // TODO: these substantially inflate the size of ComponentImpl and are almost always nullptr
  // ...should we move them to a separate allocation?
  AVLTree<SharedString, PropertyDesc> instanceProperties;
  AVLTree<SharedString, MethodDesc> instanceMethods;
  AVLTree<SharedString, EventDesc> instanceEvents;

private:
  friend class Kernel;
  friend class LuaState;
};

} // namespace ep

// HAX: this needs to be here for stupid C++ forward referencing reasons
#include "components/logger.h"

#endif // EPCOMPONENT_H
