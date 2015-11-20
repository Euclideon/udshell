
#include "ep/cpp/platform.h"

#include "kernel.h"
#include "components/logger.h"

// TODO: shut up about sprintf! **REMOVE ME**
#if defined(EP_WINDOWS)
#pragma warning(disable: 4996)
#endif // defined(EP_WINDOWS)

namespace kernel
{

void Component::Init(InitParams initParams)
{
  for (auto &kv : initParams)
  {
    const PropertyDesc *pDesc = GetPropertyDesc(kv.key.asString());
    if (pDesc && pDesc->setter)
      pDesc->setter.set(this, kv.value);
  }

/*
  // TODO: rethink this whole thing!!

  // allocate property change events
  propertyChange.length = pType->propertyTree.Size();
  propertyChange.ptr = epAllocType(Event<>, propertyChange.length, epAF_None);
  for (size_t i = 0; i<propertyChange.length; ++i)
    new(&propertyChange.ptr[i]) Event<>();
*/
  InitComplete();
}

Component::~Component()
{
  LogDebug(4, "Destroy component: {0} ({1})", uid, name);
  GetKernel().DestroyComponent(this);
}

const PropertyDesc *Component::GetPropertyDesc(String _name) const
{
  const PropertyDesc *pDesc = instanceProperties.Get(_name);
  if (!pDesc)
    pDesc = GetDescriptor()->propertyTree.Get(_name);
  return pDesc;
}
const MethodDesc *Component::GetMethodDesc(String _name) const
{
  const MethodDesc *pDesc = instanceMethods.Get(_name);
  if (!pDesc)
    pDesc = GetDescriptor()->methodTree.Get(_name);
  return pDesc;
}
const EventDesc *Component::GetEventDesc(String _name) const
{
  const EventDesc *pDesc = instanceEvents.Get(_name);
  if (!pDesc)
    pDesc = GetDescriptor()->eventTree.Get(_name);
  return pDesc;
}

const StaticFuncDesc *Component::GetStaticFuncDesc(String _name) const
{
  return GetDescriptor()->staticFuncTree.Get(_name);
}

void Component::AddDynamicProperty(const PropertyDesc &property)
{
  instanceProperties.Insert(property.id, property);
}
void Component::AddDynamicMethod(const MethodDesc &method)
{
  instanceMethods.Insert(method.id, method);
}
void Component::AddDynamicEvent(const EventDesc &event)
{
  instanceEvents.Insert(event.id, event);
}

void Component::RemoveDynamicProperty(String _name)
{
  instanceProperties.Remove(_name);
}
void Component::RemoveDynamicMethod(String _name)
{
  instanceMethods.Remove(_name);
}
void Component::RemoveDynamicEvent(String _name)
{
  instanceEvents.Remove(_name);
}

void Component::SetProperty(String property, const Variant &value)
{
  const PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc)
  {
    LogWarning(2, "No property '{0}' for component '{1}'", property, name.empty() ? uid : name);
    return;
  }
  if (!pDesc->setter || pDesc->flags & udPF_Immutable)
  {
    LogWarning(2, "Property '{0}' for component '{1}' is read-only", property, name.empty() ? uid : name);
    return;
  }
  pDesc->setter.set(this, value);
//  propertyChange[pDesc->index].Signal();
}

Variant Component::GetProperty(String property) const
{
  const PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc)
  {
    LogWarning(2, "No property '{0}' for component '{1}'", property, name.empty() ? uid : name);
    return Variant();
  }
  if (!pDesc->getter)
  {
    LogWarning(2, "Property '{0}' for component '{1}' is write-only", property, name.empty() ? uid : name);
    return Variant();
  }
  return pDesc->getter.get(this);
}

Variant Component::CallMethod(String method, Slice<const Variant> args)
{
  const MethodDesc *pDesc = GetMethodDesc(method);
  if (!pDesc)
  {
    LogWarning(1, "Method not found!");
    return Variant();
  }
  return pDesc->method.call(this, args);
}

void Component::Subscribe(String eventName, const Variant::VarDelegate &d)
{
  const EventDesc *pDesc = GetEventDesc(eventName);
  if (!pDesc)
  {
    LogWarning(2, "No event '{0}' for component '{1}'", eventName, name.empty() ? uid : name);
    return;
  }
  pDesc->ev.subscribe(this, d);
}

void Component::Unsubscribe()
{
  // TODO
}


ptrdiff_t epStringify(Slice<char> buffer, String format, const Component *pComponent, const epVarArg *pArgs)
{
#if 0 // TODO : Fix this
  ptrdiff_t len = pComponent->uid.length + 1;
  if (!buffer.ptr)
    return len;
  if (buffer.length < (size_t)len)
    return -len;

  // HACK: this could be a lot nicer!
  return epStringifyTemplate(buffer, format, MutableString<64>(Concat, "@", pComponent->uid), pArgs);
#endif
  return 0;
}

} // namespace kernel
