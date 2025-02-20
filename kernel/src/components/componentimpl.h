#pragma once
#ifndef EPCOMPONENT_H
#define EPCOMPONENT_H

#include "ep/cpp/componentdesc.h"
#include "ep/cpp/event.h"
#include "ep/cpp/internal/i/icomponent.h"
#include "ep/cpp/component/component.h"

#include <stdio.h>

namespace ep {

class ComponentImpl : public BaseImpl<Component, IComponent>
{
public:
  ComponentImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {
    // Temporary solution: get 'name' from initParams
    Variant *pName = initParams.get("name");
    if (pName)
      SetName(pName->asSharedString());
  }

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
  SubscriptionRef Subscribe(String eventName, const VarDelegate &delegate) override final;

  Variant Save() const override final { return Variant::VarMap(); }

  void AddDynamicProperty(const PropertyInfo &property, const MethodShim *pGetter, const MethodShim *pSetter) override final;
  void AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod) override final;
  void AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe) override final;
  void RemoveDynamicProperty(String name) override final;
  void RemoveDynamicMethod(String name) override final;
  void RemoveDynamicEvent(String name) override final;

  void SetUserData(void *pData) { pInstance->pUserData = pData; }

protected:
  ~ComponentImpl();

  void ReceiveMessage(String message, String sender, const Variant &data) override final;

  // property access
  size_t NumProperties() const { return instanceProperties.size() + ((const ComponentDescInl*)getDescriptor())->propertyTree.size(); }
  size_t NumMethods() const { return instanceMethods.size() + ((const ComponentDescInl*)getDescriptor())->methodTree.size(); }
  size_t NumEvents() const { return instanceEvents.size() + ((const ComponentDescInl*)getDescriptor())->eventTree.size(); }
  size_t NumStaticFuncs() const { return ((const ComponentDescInl*)getDescriptor())->staticFuncTree.size(); }

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
