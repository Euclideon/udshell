
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
  return pType->propertyTree.Get(name);
}
const MethodDesc *Component::GetMethodDesc(udString name) const
{
  return pType->methodTree.Get(name);
}
const EventDesc *Component::GetEventDesc(udString name) const
{
  return pType->eventTree.Get(name);
}

void Component::SetProperty(udString property, const udVariant &value)
{
  const PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc)
    return; // TODO: make noise "no property"
  if (!pDesc->setter || pDesc->info.flags & udPF_Immutable)
    return; // TODO: make noise "not writable"
  pDesc->setter->set(this, value);
//  propertyChange[pDesc->index].Signal();
}

udVariant Component::GetProperty(udString property) const
{
  const PropertyDesc *pDesc = GetPropertyDesc(property);
  if (!pDesc)
    return udVariant(); // TODO: make noise "no property"
  if (!pDesc->getter)
    return udVariant(); // TODO: make noise "not readable"
  return pDesc->getter->get(this);
}


udResult Component::ReceiveMessage(udString message, udString sender, const udVariant &data)
{
  if (message.eqi("set"))
  {
    udSlice<udVariant> arr = data.asArray();
    SetProperty(arr[0].asString(), arr[1]);
  }
  else if (message.eqi("get"))
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

void Component::LogError(int level, udString text) const { pKernel->LogError(level, text, uid); }
void Component::LogWarning(int level, udString text) const { pKernel->LogWarning(level, text, uid); }
void Component::LogDebug(int level, udString text) const { pKernel->LogDebug(level, text, uid); }
void Component::LogInfo(int level, udString text) const { pKernel->LogInfo(level, text, uid); }
void Component::LogScript(int level, udString text) const { pKernel->LogScript(level, text, uid); }
void Component::LogTrace(int level, udString text) const { pKernel->LogTrace(level, text, uid); }
void Component::Log(int level, udString text) const { pKernel->Log(level, text, uid); }

} // namespace ud
