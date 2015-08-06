
#include "udPlatform.h"
#include "udPlatformUtil.h"

#include "kernel.h"

// TODO: shut up about sprintf! **REMOVE ME**
#if UDPLATFORM_WINDOWS
#pragma warning(disable: 4996)
#endif // UDPLATFORM_WINDOWS

namespace ud
{

static PropertyDesc props[] =
{
  {
    {
      "uid", // id
      "UID", // displayName
      "Component UID", // description
      TypeDesc(PropertyType::String) // type
    },
    &Component::GetUid, // getter
    nullptr, // setter
  },
  {
    {
      "type", // id
      "Type", // displayName
      "Component Type", // description
      TypeDesc(PropertyType::String) // type
    },
    &Component::GetType, // getter
    nullptr, // setter
  },
  {
    {
      "displayname", // id
      "Display Name", // displayName
      "Component Display Name", // description
      TypeDesc(PropertyType::String) // type
    },
    &Component::GetDisplayName, // getter
    nullptr, // setter
  },
  {
    {
      "description", // id
      "Description", // displayName
      "Component Description", // description
      TypeDesc(PropertyType::String) // type
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

  udSlice<PropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};


void Component::Init(udInitParams initParams)
{
  for (auto &kv : initParams)
  {
    const PropertyDesc *pDesc = FindPropertyDesc(kv.key.asString());
    if (pDesc && pDesc->setter)
      pDesc->setter.set(this, kv.value);
  }

  // allocate property change events
  propertyChange.length = NumProperties();
  propertyChange.ptr = udAllocType(udEvent<>, propertyChange.length, udAF_None);
  for (size_t i = 0; i<propertyChange.length; ++i)
    new(&propertyChange.ptr[i]) udEvent<>();
}

Component::~Component()
{
  for (size_t i = 0; i<propertyChange.length; ++i)
    propertyChange.ptr[i].~udEvent<>();
  udFree(propertyChange.ptr);
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

const PropertyDesc *Component::FindPropertyDesc(udString name) const
{
  const ComponentDesc *pDesc = pType;
  while (pDesc)
  {
    for (auto &prop : pDesc->properties)
    {
      if (prop.info.id.eqi(name))
        return &prop;
    }
    pDesc = pDesc->pSuperDesc;
  }
  return nullptr;
}

const PropertyInfo *Component::GetPropertyInfo(int index) const
{
  // TODO: this is shit, traversing all properties should be constant time
  const ComponentDesc *pDesc = pType;
  while (pDesc)
  {
    for (auto &prop : pDesc->properties)
    {
      if (prop.index == index)
        return &prop.info;
    }
    pDesc = pDesc->pSuperDesc;
  }

  return nullptr;
}

const PropertyInfo *Component::GetPropertyInfo(udString property) const
{
  const PropertyDesc *pDesc = FindPropertyDesc(property);
  return pDesc ? &pDesc->info : nullptr;
}

void Component::SetProperty(udString property, const udVariant &value)
{
  const PropertyDesc *pDesc = FindPropertyDesc(property);
  if (!pDesc)
    return; // TODO: make noise
  if (!pDesc->setter)
    return; // TODO: make noise
  if (pDesc->info.flags & udPF_Immutable)
    return; // TODO: make noise
  pDesc->setter.set(this, value);
  propertyChange[pDesc->index].Signal();
}

udVariant Component::GetProperty(udString property) const
{
  const PropertyDesc *pDesc = FindPropertyDesc(property);
  if (!pDesc)
    return udVariant(); // TODO: make noise
  if (!pDesc->getter)
    return udVariant(); // TODO: make noise
  return pDesc->getter.get(this);
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

} // namespace ud
