
#include "udPlatform.h"
#include "udPlatformUtil.h"

#include "kernel.h"
#include "components/logger.h"

// TODO: shut up about sprintf! **REMOVE ME**
#if UDPLATFORM_WINDOWS
#pragma warning(disable: 4996)
#endif // UDPLATFORM_WINDOWS

namespace ud
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

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "component", // id
  "Component", // displayName
  "Is a component", // description

  udSlice<CPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};


void Component::Init(udInitParams initParams)
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
  propertyChange.ptr = udAllocType(udEvent<>, propertyChange.length, udAF_None);
  for (size_t i = 0; i<propertyChange.length; ++i)
    new(&propertyChange.ptr[i]) udEvent<>();
*/
}

Component::~Component()
{
}


bool Component::IsType(udString type) const
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

const PropertyDesc *Component::GetPropertyDesc(udString name) const
{
  const PropertyDesc *pDesc = instanceProperties.Get(name);
  if (!pDesc)
    pDesc = pType->propertyTree.Get(name);
  return pDesc;
}
const MethodDesc *Component::GetMethodDesc(udString name) const
{
  const MethodDesc *pDesc = instanceMethods.Get(name);
  if (!pDesc)
    pDesc = pType->methodTree.Get(name);
  return pDesc;
}
const EventDesc *Component::GetEventDesc(udString name) const
{
  const EventDesc *pDesc = instanceEvents.Get(name);
  if (!pDesc)
    pDesc = pType->eventTree.Get(name);
  return pDesc;
}

const StaticFuncDesc *Component::GetStaticFuncDesc(udString name) const
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

void Component::RemoveDynamicProperty(udString name)
{
  instanceProperties.Remove(name);
}
void Component::RemoveDynamicMethod(udString name)
{
  instanceMethods.Remove(name);
}
void Component::RemoveDynamicEvent(udString name)
{
  instanceEvents.Remove(name);
}

void Component::SetProperty(udString property, const udVariant &value)
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

udVariant Component::GetProperty(udString property) const
{
  const PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc)
  {
    LogWarning(2, "No property '{0}' for component '{1}'", property, name.empty() ? uid : name);
    return udVariant();
  }
  if (!pDesc->getter)
  {
    LogWarning(2, "Property '{0}' for component '{1}' is write-only", property, name.empty() ? uid : name);
    return udVariant();
  }
  return pDesc->getter->get(this);
}

udVariant Component::CallMethod(udString method, udSlice<udVariant> args)
{
  const MethodDesc *pDesc = GetMethodDesc(method);
  if (!pDesc)
  {
    LogWarning(1, "Method not found!");
    return udVariant();
  }
  return pDesc->method->call(this, args);
}

void Component::Subscribe(udString eventName, const udVariant::VarDelegate &d)
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


udResult Component::ReceiveMessage(udString message, udString sender, const udVariant &data)
{
  if (message.eqIC("set"))
  {
    udSlice<udVariant> arr = data.asArray();
    SetProperty(arr[0].asString(), arr[1]);
  }
  else if (message.eqIC("get"))
  {
    if (!sender.empty())
    {
      char mem[1024];
      udSlice<char> buffer(mem, sizeof(mem));
//      GetProperty(data, &buffer);
//      SendMessage(sender, "val", buffer);
    }
  }
  return udR_Success;
}

udResult Component::SendMessage(udString target, udString message, const udVariant &data)
{
  return pKernel->SendMessage(target, uid, message, data);
}

void Component::LogInternal(int level, udString text, int category, udString componentUID) const
{
  pKernel->GetLogger()->Log(level, text, (LogCategories)category, componentUID);
}

} // namespace ud

ptrdiff_t udStringify(udSlice<char> buffer, udString format, ud::ComponentRef spComponent, const udVarArg *pArgs)
{
  ptrdiff_t len = spComponent->uid.length + 1;
  if (!buffer.ptr)
    return len;
  if (buffer.length < (size_t)len)
    return -len;

  // HACK: this could be a lot nicer!
  udMutableString<64> uid; uid.concat("@", spComponent->uid);
  return udStringifyTemplate(buffer, format, uid, pArgs);
}
