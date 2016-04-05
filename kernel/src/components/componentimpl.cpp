
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

Array<const PropertyInfo> Component::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY_RO(Uid, "Component UID", nullptr, 0),
    EP_MAKE_PROPERTY(Name, "Component Name", nullptr, 0),
    EP_MAKE_PROPERTY_RO(Type, "Component Type", nullptr, 0),
    EP_MAKE_PROPERTY_RO(DisplayName, "Component Display Name", nullptr, 0),
    EP_MAKE_PROPERTY_RO(Description, "Component Description", nullptr, 0),
  };
}
Array<const MethodInfo> Component::GetMethods() const
{
  return{
  };
}


ComponentImpl::~ComponentImpl()
{
  pInstance->LogDebug(4, "Destroy component: {0} ({1})", pInstance->uid, pInstance->name);

  // HAX: we take access to KernelImpl; low-level Component stuff
  KernelImpl *pKernelImpl = GetKernel()->GetImpl();
  pKernelImpl->DestroyComponent(pInstance);
}

void ComponentImpl::SetName(SharedString name)
{
  // HAX: we take access to KernelImpl; low-level Component stuff
  KernelImpl *pKernelImpl = GetKernel()->GetImpl();
  if (pInstance->name)
    pKernelImpl->namedInstanceRegistry.Remove(pInstance->name);
  pInstance->name = name;
  if (name)
    pKernelImpl->namedInstanceRegistry.Insert(name, pInstance);
}

void ComponentImpl::ReceiveMessage(String message, String sender, const Variant &data)
{
  if (message.eqIC("set"))
  {
    Slice<Variant> arr = data.asArray();
    pInstance->Set(arr[0].asString(), arr[1]);
  }
  else if (message.eqIC("get"))
  {
    if (!sender.empty())
    {
      Variant p = pInstance->Get(data.asString());
      pInstance->SendMessage(sender, "val", p);
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
      props.pushBack(p.key);
  }
  return std::move(props);
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
      functions.pushBack(f.key);
    for (auto sf : GetDescriptor()->staticFuncTree)
      functions.pushBack(sf.key);
  }
  return std::move(functions);
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
      events.pushBack(e.key);
  }
  return std::move(events);
}

const PropertyDesc *ComponentImpl::GetPropertyDesc(String _name, EnumerateFlags enumerateFlags) const
{
  const PropertyDesc *pDesc = nullptr;
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
    pDesc = instanceProperties.Get(_name);
  if (!pDesc && !(enumerateFlags & EnumerateFlags::NoStatic))
    pDesc = ((const ComponentDescInl*)GetDescriptor())->propertyTree.Get(_name);
  return pDesc;
}
const MethodDesc *ComponentImpl::GetMethodDesc(String _name, EnumerateFlags enumerateFlags) const
{
  const MethodDesc *pDesc = nullptr;
  if (!(enumerateFlags & EnumerateFlags::NoDynamic))
    pDesc = instanceMethods.Get(_name);
  if (!pDesc && !(enumerateFlags & EnumerateFlags::NoStatic))
    pDesc = ((const ComponentDescInl*)GetDescriptor())->methodTree.Get(_name);
  return pDesc;
}
const EventDesc *ComponentImpl::GetEventDesc(String _name, EnumerateFlags enumerateFlags) const
{
  const EventDesc *pDesc = nullptr;
  if(!(enumerateFlags & EnumerateFlags::NoDynamic))
    pDesc = instanceEvents.Get(_name);
  if (!pDesc && !(enumerateFlags & EnumerateFlags::NoStatic))
    pDesc = ((const ComponentDescInl*)GetDescriptor())->eventTree.Get(_name);
  return pDesc;
}
const StaticFuncDesc *ComponentImpl::GetStaticFuncDesc(String _name, EnumerateFlags enumerateFlags) const
{
  return ((const ComponentDescInl*)GetDescriptor())->staticFuncTree.Get(_name);
}

void ComponentImpl::AddDynamicProperty(const PropertyInfo &property, const MethodShim *pGetter, const MethodShim *pSetter)
{
  PropertyDesc desc(property, pGetter ? *pGetter : MethodShim(property.pGetterMethod), pSetter ? *pSetter : MethodShim(property.pSetterMethod));
  instanceProperties.Insert(desc.id, desc);
}
void ComponentImpl::AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod)
{
  MethodDesc desc(method, pMethod ? *pMethod : MethodShim(method.pMethod));
  instanceMethods.Insert(desc.id, desc);
}
void ComponentImpl::AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe)
{
  EventDesc desc(event, pSubscribe ? *pSubscribe : EventShim(event.pSubscribe));
  instanceEvents.Insert(desc.id, desc);
}

void ComponentImpl::RemoveDynamicProperty(String _name)
{
  instanceProperties.Remove(_name);
}
void ComponentImpl::RemoveDynamicMethod(String _name)
{
  instanceMethods.Remove(_name);
}
void ComponentImpl::RemoveDynamicEvent(String _name)
{
  instanceEvents.Remove(_name);
}

Variant ComponentImpl::Get(String property) const
{
  const PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc || !pDesc->getter)
  {
    // TODO: throw in this case?
    const char *pMessage = pDesc ? "Property '{0}' for component '{1}' is write-only" : "No property '{0}' for component '{1}'";
    pInstance->LogWarning(2, pMessage, property, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return Variant();
  }
  Variant r = pDesc->getter.get(pInstance);
  r.throwError();
  return std::move(r);
}
void ComponentImpl::Set(String property, const Variant &value)
{
  const PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc || !pDesc->setter || pDesc->flags & epPF_Immutable)
  {
    // TODO: throw in this case?
    const char *pMessage = pDesc ? "Property '{0}' for component '{1}' is read-only" : "No property '{0}' for component '{1}'";
    pInstance->LogWarning(2, pMessage, property, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return;
  }
  pDesc->setter.set(pInstance, value);
  if (ErrorLevel())
    throw GetError();
  // TODO: should properties have implicit signals?
  //  propertyChange[pDesc->index].Signal();
}

Variant ComponentImpl::Call(String method, Slice<const Variant> args)
{
  const MethodDesc *pDesc = GetMethodDesc(method);
  if (!pDesc)
  {
    const StaticFuncDesc *pFunc = GetStaticFuncDesc(method);
    if (!pFunc)
    {
      // TODO: throw in this case?
      pInstance->LogWarning(1, "Method '{0}' not found!", method);
      return Variant();
    }
    Variant r2 = pFunc->staticFunc.call(args);
    r2.throwError();
    return std::move(r2);
  }
  Variant r = pDesc->method.call(pInstance, args);
  r.throwError();
  return std::move(r);
}

void ComponentImpl::Subscribe(String eventName, const VarDelegate &d)
{
  const EventDesc *pDesc = GetEventDesc(eventName);
  if (!pDesc)
  {
    pInstance->LogWarning(2, "No event '{0}' for component '{1}'", eventName, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return;
  }
  pDesc->ev.subscribe(pInstance, d);
  if (ErrorLevel())
    throw GetError();
}

} // namespace ep
