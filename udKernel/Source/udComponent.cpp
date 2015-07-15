
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
    udPropertyType::String, // type
    0, // arrayLength
    udPF_NoWrite, // flags
    udPropertyDisplayType::Default, // displayType
    udGetter(&udComponent::getUid),
    nullptr
  },
  {
    "type", // id
    "Type", // displayName
    "Component Type", // description
    udPropertyType::String, // type
    0, // arrayLength
    udPF_NoWrite, // flags
    udPropertyDisplayType::Default, // displayType
    udGetter(&udComponent::getType),
    nullptr
  },
  {
    "displayname", // id
    "Display Name", // displayName
    "Component Display Name", // description
    udPropertyType::String, // type
    0, // arrayLength
    udPF_NoWrite, // flags
    udPropertyDisplayType::Default, // displayType
    udGetter(&udComponent::getDisplayName),
    nullptr
  },
  {
    "description", // id
    "Description", // displayName
    "Component Description", // description
    udPropertyType::String, // type
    0, // arrayLength
    udPF_NoWrite, // flags
    udPropertyDisplayType::Default, // displayType
    udGetter(&udComponent::getDescription),
    nullptr
  }
};
const udComponentDesc udComponent::descriptor =
{
  nullptr, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "component", // id
  "udComponent",    // displayName
  "Is a component", // description

  nullptr, // pInit
  nullptr, // pInitRender
  nullptr, // pCreateInstance

  udSlice<const udPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};


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

static size_t BytesForArray(udPropertyType type, size_t count)
{
  switch (type)
  {
    case udPropertyType::Boolean: return sizeof(bool)*count;
    case udPropertyType::Integer: return sizeof(int64_t)*count;
    case udPropertyType::Float: return sizeof(double)*count;
    case udPropertyType::String: return sizeof(udString)*count;
    case udPropertyType::Component: return sizeof(udComponentRef)*count;
    default:
      UDASSERT(false, "Shouldn't be here?");
  }
  return 0;
}

void udComponent::SetProperty(udString property, const udVariant value)
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return; // TODO: make noise
  if (pDesc->flags & udPropertyFlags::udPF_NoWrite)
    return; // TODO: make noise
  pDesc->setter.set(this, value);
}

udVariant udComponent::GetProperty(udString property) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udVariant(); // TODO: make noise
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udVariant(); // TODO: make noise
  return pDesc->getter.get(this);
}


udResult udComponent::ReceiveMessage(udString message, udString sender, udVariant data)
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

udResult udComponent::SendMessage(udString target, udString message, udVariant data)
{
  return pKernel->SendMessage(target, uid, message, data);
}
