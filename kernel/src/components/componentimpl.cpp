
#include "ep/cpp/platform.h"
#include "ep/cpp/componentdesc.h"

#include "kernel.h"
#include "componentdesc.h"
#include "components/logger.h"

// TODO: shut up about sprintf! **REMOVE ME**
#if defined(EP_WINDOWS)
#pragma warning(disable: 4996)
#endif // defined(EP_WINDOWS)

namespace ep {

ComponentImpl::~ComponentImpl()
{
  pInstance->LogDebug(4, "Destroy component: {0} ({1})", pInstance->uid, pInstance->name);
  GetKernel()->DestroyComponent(pInstance);
}

void ComponentImpl::SetName(SharedString name)
{
  kernel::Kernel *pKernel = (kernel::Kernel*)pInstance->pKernel;
  if (pInstance->name)
    pKernel->namedInstanceRegistry.Remove(pInstance->name);
  pInstance->name = name;
  if (name)
    pKernel->namedInstanceRegistry.Insert(name, pInstance);
}

void ComponentImpl::Init(Variant::VarMap initParams)
{
  // Temporary solution: get 'name' from initParams
  Variant *pName = initParams.Get("name");
  if (pName)
    SetName(pName->asSharedString());

  // TODO uncomment this when we've thought it through
  /*
  for (auto kv : initParams)
  {
    const PropertyDesc *pDesc = GetPropertyDesc(kv.key.asString());
    if (pDesc && pDesc->setter)
      pDesc->setter.set(this, kv.value);
  }
  */

/*
  // TODO: rethink this whole thing!!

  // allocate property change events
  propertyChange.length = pType->propertyTree.Size();
  propertyChange.ptr = epAllocType(Event<>, propertyChange.length, epAF_None);
  for (size_t i = 0; i<propertyChange.length; ++i)
    new(&propertyChange.ptr[i]) Event<>();
*/
  pInstance->InitComplete();
}

epResult ComponentImpl::ReceiveMessage(String message, String sender, const Variant &data)
{
  if (message.eqIC("set"))
  {
    Slice<Variant> arr = data.asArray();
    pInstance->SetProperty(arr[0].asString(), arr[1]);
  }
  else if (message.eqIC("get"))
  {
    if (!sender.empty())
    {
      Variant p = pInstance->GetProperty(data.asString());
      pInstance->SendMessage(sender, "val", p);
    }
  }
  return epR_Success;
}

const kernel::PropertyDesc *ComponentImpl::GetPropertyDesc(String _name) const
{
  const kernel::PropertyDesc *pDesc = instanceProperties.Get(_name);
  if (!pDesc)
    pDesc = GetDescriptor()->propertyTree.Get(_name);
  return pDesc;
}
const kernel::MethodDesc *ComponentImpl::GetMethodDesc(String _name) const
{
  const kernel::MethodDesc *pDesc = instanceMethods.Get(_name);
  if (!pDesc)
    pDesc = GetDescriptor()->methodTree.Get(_name);
  return pDesc;
}
const kernel::EventDesc *ComponentImpl::GetEventDesc(String _name) const
{
  const kernel::EventDesc *pDesc = instanceEvents.Get(_name);
  if (!pDesc)
    pDesc = GetDescriptor()->eventTree.Get(_name);
  return pDesc;
}

const kernel::StaticFuncDesc *ComponentImpl::GetStaticFuncDesc(String _name) const
{
  return GetDescriptor()->staticFuncTree.Get(_name);
}

void ComponentImpl::AddDynamicProperty(const PropertyInfo &property, const GetterShim *pGetter, const SetterShim *pSetter)
{
  kernel::PropertyDesc desc(property, pGetter ? *pGetter : GetterShim(property.pGetterMethod), pSetter ? *pSetter : SetterShim(property.pSetterMethod));
  instanceProperties.Insert(desc.id, desc);
}
void ComponentImpl::AddDynamicMethod(const MethodInfo &method, const MethodShim *pMethod)
{
  kernel::MethodDesc desc(method, pMethod ? *pMethod : MethodShim(method.pMethod));
  instanceMethods.Insert(desc.id, desc);
}
void ComponentImpl::AddDynamicEvent(const EventInfo &event, const EventShim *pSubscribe)
{
  kernel::EventDesc desc(event, pSubscribe ? *pSubscribe : EventShim(event.pSubscribe));
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

void ComponentImpl::SetProperty(String property, const Variant &value)
{
  const kernel::PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc)
  {
    // TODO: throw in this case?
    pInstance->LogWarning(2, "No property '{0}' for component '{1}'", property, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return;
  }
  if (!pDesc->setter || pDesc->flags & epPF_Immutable)
  {
    // TODO: throw in this case?
    pInstance->LogWarning(2, "Property '{0}' for component '{1}' is read-only", property, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return;
  }
  pDesc->setter.set(pInstance, value);
  if (ErrorLevel())
    throw GetError();
  // TODO: should properties have implicit signals?
//  propertyChange[pDesc->index].Signal();
}

Variant ComponentImpl::GetProperty(String property) const
{
  const kernel::PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc)
  {
    // TODO: throw in this case?
    pInstance->LogWarning(2, "No property '{0}' for component '{1}'", property, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return Variant();
  }
  if (!pDesc->getter)
  {
    // TODO: throw in this case?
    pInstance->LogWarning(2, "Property '{0}' for component '{1}' is write-only", property, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return Variant();
  }
  Variant r = pDesc->getter.get(pInstance);
  r.throwError();
  return std::move(r);
}

Variant ComponentImpl::CallMethod(String method, Slice<const Variant> args)
{
  const kernel::MethodDesc *pDesc = GetMethodDesc(method);
  if (!pDesc)
  {
    const kernel::StaticFuncDesc *pFunc = GetStaticFuncDesc(method);
    if (!pFunc)
    {
      // TODO: throw in this case?
      pInstance->LogWarning(1, "Method not found!");
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

void ComponentImpl::Subscribe(String eventName, const Variant::VarDelegate &d)
{
  const kernel::EventDesc *pDesc = GetEventDesc(eventName);
  if (!pDesc)
  {
    pInstance->LogWarning(2, "No event '{0}' for component '{1}'", eventName, pInstance->name.empty() ? pInstance->uid : pInstance->name);
    return;
  }
  pDesc->ev.subscribe(pInstance, d);
}

} // namespace ep
