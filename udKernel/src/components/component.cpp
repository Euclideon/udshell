
#include "udPlatform.h"
#include "udPlatformUtil.h"
#include "udKernel.h"

// TODO: shut up about sprintf! **REMOVE ME**
#if UDPLATFORM_WINDOWS
#pragma warning(disable: 4996)
#endif // UDPLATFORM_WINDOWS

namespace udKernel
{

static PropertyDesc props[] =
{
  {
    "uid", // id
    "UID", // displayName
    "Component UID", // description
    &Component::GetUid, // getter
    nullptr, // setter
    TypeDesc(PropertyType::String) // type
  },
  {
    "type", // id
    "Type", // displayName
    "Component Type", // description
    &Component::GetType, // getter
    nullptr, // setter
    TypeDesc(PropertyType::String) // type
  },
  {
    "displayname", // id
    "Display Name", // displayName
    "Component Display Name", // description
    &Component::GetDisplayName, // getter
    nullptr, // setter
    TypeDesc(PropertyType::String) // type
  },
  {
    "description", // id
    "Description", // displayName
    "Component Description", // description
    &Component::GetDescription, // getter
    nullptr, // setter
    TypeDesc(PropertyType::String) // type
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

  nullptr, // pInit
  nullptr, // pCreateInstance

  udSlice<PropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};


void Component::Init(InitParams initParams)
{
  for (auto &kv : initParams)
  {
    const PropertyDesc *pDesc = FindProperty(kv.key.asString());
    if (pDesc && pDesc->setter)
      pDesc->setter.set(this, kv.value);
  }

  // allocate property change events
  size_t numProps = NumProperties();
  pPropertyChange = udAllocType(Event<>, numProps, udAF_None);
  for (size_t i = 0; i<numProps; ++i)
    new(&pPropertyChange[i]) Event<>();
}

Component::~Component()
{
  size_t numProps = NumProperties();
  for (size_t i = 0; i<numProps; ++i)
    pPropertyChange[i].~Event<>();
  udFree(pPropertyChange);
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

const PropertyDesc *Component::FindProperty(udString name) const
{
  const ComponentDesc *pDesc = pType;
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

void Component::SetProperty(udString property, const Variant &value)
{
  const PropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return; // TODO: make noise
  if (!pDesc->setter)
    return; // TODO: make noise
  if (pDesc->flags & udPF_Immutable)
    return; // TODO: make noise
  pDesc->setter.set(this, value);
  pPropertyChange[pDesc->index].Signal();
}

Variant Component::GetProperty(udString property) const
{
  const PropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return Variant(); // TODO: make noise
  if (!pDesc->getter)
    return Variant(); // TODO: make noise
  return pDesc->getter.get(this);
}


udResult Component::ReceiveMessage(udString message, udString sender, const Variant &data)
{
  if (message.eqi("set"))
  {
    udSlice<Variant> arr = data.asArray();
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

udResult Component::SendMessage(udString target, udString message, const Variant &data)
{
  return pKernel->SendMessage(target, uid, message, data);
}
} // namespace udKernel
