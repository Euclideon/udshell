
#include "ep/cpp/platform.h"

#include "kernel.h"
#include "components/logger.h"

// TODO: shut up about sprintf! **REMOVE ME**
#if defined(EP_WINDOWS)
#pragma warning(disable: 4996)
#endif // defined(EP_WINDOWS)

namespace ep
{

static CPropertyDesc props[] =
{
  {
    {
      "uid", // id
      "UID", // displayName
      "Component UID", // description
    },
    &Component::GetUid, // getter
    nullptr, // setter
  },
  {
    {
      "name", // id
      "Name", // displayName
      "Component Name", // description
    },
    &Component::GetName, // getter
    &Component::SetName, // setter
  },
  {
    {
      "type", // id
      "Type", // displayName
      "Component Type", // description
    },
    &Component::GetType, // getter
    nullptr, // setter
  },
  {
    {
      "displayname", // id
      "Display Name", // displayName
      "Component Display Name", // description
    },
    &Component::GetDisplayName, // getter
    nullptr, // setter
  },
  {
    {
      "description", // id
      "Description", // displayName
      "Component Description", // description
    },
    &Component::GetDescription, // getter
    nullptr, // setter
  }
};
ComponentDesc Component::descriptor =
{
  nullptr, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "component", // id
  "Component", // displayName
  "Is a component", // description

  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};


void Component::Init(InitParams initParams)
{
  for (auto &kv : initParams)
  {
    const PropertyDesc *pDesc = GetPropertyDesc(kv.key.asString());
    if (pDesc && *pDesc->setter)
      pDesc->setter->set(this, kv.value);
  }

/*
  // TODO: rethink this whole thing!!

  // allocate property change events
  propertyChange.length = pType->propertyTree.Size();
  propertyChange.ptr = epAllocType(Event<>, propertyChange.length, udAF_None);
  for (size_t i = 0; i<propertyChange.length; ++i)
    new(&propertyChange.ptr[i]) Event<>();
*/
  InitComplete();
}

Component::~Component()
{
  LogDebug(4, "Destroy component: {0} ({1})", uid, name);
  pKernel->DestroyComponent(this);
}


bool Component::IsType(String type) const
{
  const ComponentDesc *pDesc = pType;
  while (pDesc)
  {
    if (pType->id.eq(type))
      return true;
    pDesc = pDesc->pSuperDesc;
  }
  return false;
}

const PropertyDesc *Component::GetPropertyDesc(String name) const
{
  const PropertyDesc *pDesc = instanceProperties.Get(name);
  if (!pDesc)
    pDesc = pType->propertyTree.Get(name);
  return pDesc;
}
const MethodDesc *Component::GetMethodDesc(String name) const
{
  const MethodDesc *pDesc = instanceMethods.Get(name);
  if (!pDesc)
    pDesc = pType->methodTree.Get(name);
  return pDesc;
}
const EventDesc *Component::GetEventDesc(String name) const
{
  const EventDesc *pDesc = instanceEvents.Get(name);
  if (!pDesc)
    pDesc = pType->eventTree.Get(name);
  return pDesc;
}

const StaticFuncDesc *Component::GetStaticFuncDesc(String name) const
{
  return pType->staticFuncTree.Get(name);
}

void Component::AddDynamicProperty(const PropertyDesc &property)
{
  instanceProperties.Insert(property.info.id, property);
}
void Component::AddDynamicMethod(const MethodDesc &method)
{
  instanceMethods.Insert(method.info.id, method);
}
void Component::AddDynamicEvent(const EventDesc &event)
{
  instanceEvents.Insert(event.info.id, event);
}

void Component::RemoveDynamicProperty(String name)
{
  instanceProperties.Remove(name);
}
void Component::RemoveDynamicMethod(String name)
{
  instanceMethods.Remove(name);
}
void Component::RemoveDynamicEvent(String name)
{
  instanceEvents.Remove(name);
}

void Component::SetProperty(String property, const Variant &value)
{
  const PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc)
  {
    LogWarning(2, "No property '{0}' for component '{1}'", property, name.empty() ? uid : name);
    return;
  }
  if (!pDesc->setter || pDesc->info.flags & udPF_Immutable)
  {
    LogWarning(2, "Property '{0}' for component '{1}' is read-only", property, name.empty() ? uid : name);
    return;
  }
  pDesc->setter->set(this, value);
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
  return pDesc->getter->get(this);
}

Variant Component::CallMethod(String method, Slice<Variant> args)
{
  const MethodDesc *pDesc = GetMethodDesc(method);
  if (!pDesc)
  {
    LogWarning(1, "Method not found!");
    return Variant();
  }
  return pDesc->method->call(this, args);
}

void Component::Subscribe(String eventName, const Variant::VarDelegate &d)
{
  const EventDesc *pDesc = GetEventDesc(eventName);
  if (!pDesc)
  {
    LogWarning(2, "No event '{0}' for component '{1}'", eventName, name.empty() ? uid : name);
    return;
  }
  pDesc->ev->subscribe(ComponentRef(this), d);
}

void Component::Unsubscribe()
{
  // TODO
}


epResult Component::ReceiveMessage(String message, String sender, const Variant &data)
{
  if (message.eqIC("set"))
  {
    Slice<Variant> arr = data.asArray();
    SetProperty(arr[0].asString(), arr[1]);
  }
  else if (message.eqIC("get"))
  {
    if (!sender.empty())
    {
      char mem[1024];
      Slice<char> buffer(mem, sizeof(mem));
//      GetProperty(data, &buffer);
//      SendMessage(sender, "val", buffer);
    }
  }
  return epR_Success;
}

epResult Component::SendMessage(String target, String message, const Variant &data)
{
  return pKernel->SendMessage(target, uid, message, data);
}

void Component::LogInternal(int level, String text, int category, String componentUID) const
{
  pKernel->GetLogger()->Log(level, text, (LogCategories)category, componentUID);
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
  MutableString<64> uid; uid.concat("@", pComponent->uid);
  return epStringifyTemplate(buffer, format, uid, pArgs);
#endif
  return 0;
}

} // namespace ep
