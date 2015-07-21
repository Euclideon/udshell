
#include "udPlatform.h"
#include "udPlatformUtil.h"
#include "udKernel.h"

// TODO: shut up about sprintf! **REMOVE ME**
#if UDPLATFORM_WINDOWS
#pragma warning(disable: 4996)
#endif // UDPLATFORM_WINDOWS



static const udPropertyDesc props[] =
{
  {
    "uid", // id
    "UID", // displayName
    "Component UID", // description
    &udComponent::GetUid, // getter
    nullptr, // setter
    udTypeDesc(udPropertyType::String) // type
  },
  {
    "type", // id
    "Type", // displayName
    "Component Type", // description
    &udComponent::GetType, // getter
    nullptr, // setter
    udTypeDesc(udPropertyType::String) // type
  },
  {
    "displayname", // id
    "Display Name", // displayName
    "Component Display Name", // description
    &udComponent::GetDisplayName, // getter
    nullptr, // setter
    udTypeDesc(udPropertyType::String) // type
  },
  {
    "description", // id
    "Description", // displayName
    "Component Description", // description
    &udComponent::GetDescription, // getter
    nullptr, // setter
    udTypeDesc(udPropertyType::String) // type
  }
};
const udComponentDesc udComponent::descriptor =
{
  nullptr, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "component", // id
  "udComponent", // displayName
  "Is a component", // description

  nullptr, // pInit
  nullptr, // pCreateInstance

  udSlice<const udPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};


void udComponent::Init(udInitParams initParams)
{
  for (auto &kv : initParams)
  {
    const udPropertyDesc *pDesc = FindProperty(kv.key.asString());
    if (pDesc && pDesc->setter)
      pDesc->setter.set(this, kv.value);
  }
}


bool udComponent::IsType(udString type) const
{
  const udComponentDesc *pDesc = pType;
  while (pDesc)
  {
    if (pType->id.eq(type))
      return true;
    pDesc = pDesc->pSuperDesc;
  }
  return false;
}

const udPropertyDesc *udComponent::FindProperty(udString name) const
{
  const udComponentDesc *pDesc = pType;
  while (pDesc)
  {
    for (auto &prop : pDesc->properties)
    {
      if (prop.id.eqi(name))
        return &prop;
    }
    pDesc = pDesc->pSuperDesc;
  }
  return nullptr;
}

void udComponent::SetProperty(udString property, const udVariant &value)
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return; // TODO: make noise
  if (!pDesc->setter)
    return; // TODO: make noise
  if (pDesc->flags & udPF_Immutable)
    return; // TODO: make noise
  pDesc->setter.set(this, value);
}

udVariant udComponent::GetProperty(udString property) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udVariant(); // TODO: make noise
  if (!pDesc->getter)
    return udVariant(); // TODO: make noise
  return pDesc->getter.get(this);
}


udResult udComponent::ReceiveMessage(udString message, udString sender, const udVariant &data)
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

udResult udComponent::SendMessage(udString target, udString message, const udVariant &data)
{
  return pKernel->SendMessage(target, uid, message, data);
}
