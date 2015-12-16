#pragma once
#ifndef EPCOMPONENT_H
#define EPCOMPONENT_H

#include "componentdesc.h"
#include "ep/cpp/event.h"
#include "ep/cpp/interface/icomponent.h"
#include "ep/cpp/component.h"

#include "udHashMap.h"

#include <stdio.h>

#if defined(SendMessage)
# undef SendMessage
#endif

namespace ep {

class ComponentImpl : public BaseImpl<Component, IComponent>
{
public:
  ComponentImpl(Component *pInstance, Variant::VarMap initParams)
    : Super(pInstance) {}

  void SetName(SharedString name) override final;

  Variant GetProperty(String property) const override final;
  void SetProperty(String property, const Variant &value) override final;
  Variant CallMethod(String method, Slice<const Variant> args) override final;
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
  epResult InitComplete() override final { return epR_Success; }

  epResult ReceiveMessage(String message, String sender, const Variant &data) override final;

  // property access
  size_t NumProperties() const { return instanceProperties.Size() + GetDescriptor()->propertyTree.Size(); }
  size_t NumMethods() const { return instanceMethods.Size() + GetDescriptor()->methodTree.Size(); }
  size_t NumEvents() const { return instanceEvents.Size() + GetDescriptor()->eventTree.Size(); }
  size_t NumStaticFuncs() const { return GetDescriptor()->staticFuncTree.Size(); }

  const kernel::PropertyDesc *GetPropertyDesc(String name) const;
  const kernel::MethodDesc *GetMethodDesc(String name) const;
  const kernel::EventDesc *GetEventDesc(String name) const;
  const kernel::StaticFuncDesc *GetStaticFuncDesc(String name) const;

  // TODO: these substantially inflate the size of base Component and are almost always nullptr
  // ...should we move them to a separate allocation?
  AVLTree<SharedString, kernel::PropertyDesc> instanceProperties;
  AVLTree<SharedString, kernel::MethodDesc> instanceMethods;
  AVLTree<SharedString, kernel::EventDesc> instanceEvents;

private:
  friend class Kernel;
  friend class LuaState;
};

} // namespace ep

// HAX: this needs to be here for stupid C++ forward referencing reasons
#include "components/logger.h"

#endif // EPCOMPONENT_H
