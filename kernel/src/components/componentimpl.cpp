
#include "ep/cpp/platform.h"

#include "ep/cpp/kernel.h"
#include "componentimpl.h"
#include "kernelimpl.h"
#include "components/logger.h"

// TODO: shut up about sprintf! **REMOVE ME**
#if defined(EP_WINDOWS)
#pragma warning(disable: 4996)
#endif // defined(EP_WINDOWS)

namespace ep {

Array<const PropertyInfo> Component::getProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO("uid", getUid, "Component UID", nullptr, 0),
    EP_MAKE_PROPERTY("name", getName, setName, "Component Name", nullptr, 0),
    EP_MAKE_PROPERTY_RO("type", getType, "Component Type", nullptr, 0),
//    EP_MAKE_PROPERTY_RO("displayName", getDisplayName, "Component Display Name", nullptr, 0),  // TODO: add this back at some point?
    EP_MAKE_PROPERTY_RO("description", getDescription, "Component Description", nullptr, 0),
  };
}
Array<const MethodInfo> Component::getMethods() const
{
  return{
  };
}


ComponentImpl::~ComponentImpl()
{
  pInstance->logDebug(4, "Destroy component: {0} ({1})", pInstance->uid, pInstance->name);

  // HAX: we take access to KernelImpl; low-level Component stuff
  KernelImpl *pKernelImpl = GetKernel()->GetImpl();
  pKernelImpl->DestroyComponent(pInstance);
}

void ComponentImpl::SetName(SharedString name)
{
  // HAX: we take access to KernelImpl; low-level Component stuff
  KernelImpl *pKernelImpl = GetKernel()->GetImpl();

  if (name && pKernelImpl->namedInstanceRegistry.exists(name))
    EPTHROW_WARN(Result::AlreadyExists, 1, "Name is already in use");

  if (pInstance->name)
    pKernelImpl->namedInstanceRegistry.remove(pInstance->name);
  pInstance->name = name;
  if (name)
    pKernelImpl->namedInstanceRegistry.insert(name, pInstance);
}

void ComponentImpl::ReceiveMessage(String message, String sender, const Variant &data)
{
  if (message.eqIC("set"))
  {
    Slice<Variant> arr = data.asArray();
    pInstance->set(arr[0].asString(), arr[1]);
  }
  else if (message.eqIC("get"))
  {
    if (!sender.empty())
    {
      Variant p = pInstance->get(data.asString());
      pInstance->sendMessage(sender, "val", p);
    }
  }
}

Array<SharedString> ComponentImpl::EnumerateProperties(EnumerateFlags enumerateFlags) const
{
  Array<SharedString> props;
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
  {
    for (auto ip : instanceProperties)
      props.pushBack(ip.key);
  }
  if (!(enumerateFlags & EnumerateFlags::NoStatic))
  {
    for (auto p : GetDescriptor()->propertyTree)
    {
      if (!(enumerateFlags & EnumerateFlags::NoInherited) || !GetSuperDescriptor()->propertyTree.get(p.key))
        props.pushBack(p.key);
    }
  }
  return props;
}
Array<SharedString> ComponentImpl::EnumerateFunctions(EnumerateFlags enumerateFlags) const
{
  Array<SharedString> functions;
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
  {
    for (auto ifunc : instanceMethods)
      functions.pushBack(ifunc.key);
  }
  if (!(enumerateFlags & EnumerateFlags::NoStatic))
  {
    for (auto f : GetDescriptor()->methodTree)
    {
      if (!(enumerateFlags & EnumerateFlags::NoInherited) || !GetSuperDescriptor()->methodTree.get(f.key))
        functions.pushBack(f.key);
    }
    for (auto sf : GetDescriptor()->staticFuncTree)
    {
      if (!(enumerateFlags & EnumerateFlags::NoInherited) || !GetSuperDescriptor()->staticFuncTree.get(sf.key))
        functions.pushBack(sf.key);
    }
  }
  return functions;
}
Array<SharedString> ComponentImpl::EnumerateEvents(EnumerateFlags enumerateFlags) const
{
  Array<SharedString> events;
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
  {
    for (auto ie : instanceEvents)
      events.pushBack(ie.key);
  }
  if (!(enumerateFlags & EnumerateFlags::NoStatic))
  {
    for (auto e : GetDescriptor()->eventTree)
    {
      if (!(enumerateFlags & EnumerateFlags::NoInherited) || !GetSuperDescriptor()->eventTree.get(e.key))
        events.pushBack(e.key);
    }
  }
  return events;
}

const PropertyDesc *ComponentImpl::GetPropertyDesc(String _name, EnumerateFlags enumerateFlags) const
{
  const PropertyDesc *pDesc = nullptr;
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
    pDesc = instanceProperties.get(_name);
  if (!pDesc && !(enumerateFlags & EnumerateFlags::NoStatic))
  {
    if (!(enumerateFlags & EnumerateFlags::NoInherited) || !GetSuperDescriptor()->propertyTree.get(_name))
      pDesc = ((const ComponentDescInl*)GetDescriptor())->propertyTree.get(_name);
  }
  return pDesc;
}
const MethodDesc *ComponentImpl::GetMethodDesc(String _name, EnumerateFlags enumerateFlags) const
{
  const MethodDesc *pDesc = nullptr;
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
    pDesc = instanceMethods.get(_name);
  if (!pDesc && !(enumerateFlags & EnumerateFlags::NoStatic))
  {
    if (!(enumerateFlags & EnumerateFlags::NoInherited) || !GetSuperDescriptor()->methodTree.get(_name))
      pDesc = ((const ComponentDescInl*)GetDescriptor())->methodTree.get(_name);
  }
  return pDesc;
}
const EventDesc *ComponentImpl::GetEventDesc(String _name, EnumerateFlags enumerateFlags) const
{
  const EventDesc *pDesc = nullptr;
  if(!(enumerateFlags & EnumerateFlags::NoDynamic))
    pDesc = instanceEvents.get(_name);
  if (!pDesc && !(enumerateFlags & EnumerateFlags::NoStatic))
  {
    if (!(enumerateFlags & EnumerateFlags::NoInherited) || !GetSuperDescriptor()->eventTree.get(_name))
      pDesc = ((const ComponentDescInl*)GetDescriptor())->eventTree.get(_name);
  }
  return pDesc;
}
const StaticFuncDesc *ComponentImpl::GetStaticFuncDesc(String _name, EnumerateFlags enumerateFlags) const
{
  if (!(enumerateFlags & EnumerateFlags::NoInherited) || !GetSuperDescriptor()->staticFuncTree.get(_name))
    return ((const ComponentDescInl*)GetDescriptor())->staticFuncTree.get(_name);
  return nullptr;
}

void ComponentImpl::AddDynamicProperty(const PropertyInfo &property, const MethodShim *pGetter, const MethodShim *pSetter)
{
  PropertyDesc desc(property, pGetter ? *pGetter : MethodShim(property.pGetterMethod), pSetter ? *pSetter : MethodShim(property.pSetterMethod));
  instanceProperties.insert(desc.id, desc);
}
void ComponentImpl::AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod)
{
  MethodDesc desc(method, pMethod ? *pMethod : MethodShim(method.pMethod));
  instanceMethods.insert(desc.id, desc);
}
void ComponentImpl::AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe)
{
  EventDesc desc(event, pSubscribe ? *pSubscribe : EventShim(event.pSubscribe));
  instanceEvents.insert(desc.id, desc);
}

void ComponentImpl::RemoveDynamicProperty(String _name)
{
  instanceProperties.remove(_name);
}
void ComponentImpl::RemoveDynamicMethod(String _name)
{
  instanceMethods.remove(_name);
}
void ComponentImpl::RemoveDynamicEvent(String _name)
{
  instanceEvents.remove(_name);
}

Variant ComponentImpl::Get(String property) const
{
  const PropertyDesc *pDesc = pInstance->getPropertyDesc(property);
  if (!pDesc || !pDesc->getter)
  {
    // TODO: throw in this case?
    const char *pMessage = pDesc ? "Property '{0}' for component '{1}' is write-only" : "No property '{0}' for component '{1}'";
    pInstance->logWarning(2, pMessage, property, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return Variant();
  }
  Variant r = pDesc->getter.get(pInstance);
  r.throwError();
  return r;
}
void ComponentImpl::Set(String property, const Variant &value)
{
  const PropertyDesc *pDesc = pInstance->getPropertyDesc(property);
  if (!pDesc || !pDesc->setter || pDesc->flags & epPF_Immutable)
  {
    // TODO: throw in this case?
    const char *pMessage = pDesc ? "Property '{0}' for component '{1}' is read-only" : "No property '{0}' for component '{1}'";
    pInstance->logWarning(2, pMessage, property, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return;
  }

  Variant r = pDesc->setter.set(pInstance, value);
  r.throwError();

  // TODO: should properties have implicit signals?
  //  propertyChange[pDesc->index].Signal();
}

Variant ComponentImpl::Call(String method, Slice<const Variant> args)
{
  const MethodDesc *pDesc = pInstance->getMethodDesc(method);
  if (!pDesc)
  {
    const StaticFuncDesc *pFunc = pInstance->getStaticFuncDesc(method);
    if (!pFunc)
    {
      // TODO: throw in this case?
      pInstance->logWarning(1, "Method '{0}' not found!", method);
      return Variant();
    }
    Variant r2 = pFunc->staticFunc.call(args);
    r2.throwError();
    return r2;
  }
  Variant r = pDesc->method.call(pInstance, args);
  r.throwError();
  return r;
}

SubscriptionRef ComponentImpl::Subscribe(String eventName, const VarDelegate &d)
{
  const EventDesc *pDesc = pInstance->getEventDesc(eventName);
  if (!pDesc)
  {
    pInstance->logWarning(2, "No event '{0}' for component '{1}'", eventName, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return nullptr;
  }

  Variant r = pDesc->ev.subscribe(pInstance, d);
  r.throwError();
  return r.asSubscription();
}

} // namespace ep
