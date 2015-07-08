
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

  udSlice<const udPropertyDesc>(props, ARRAY_LENGTH(props)) // propeties
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

udResult udComponent::SetProperty(udString property, udVariant value)
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->flags & udPropertyFlags::udPF_NoWrite)
    return udR_Failure_; // TODO: return better error message

  if (pDesc->arrayLength == 0)
  {
    if (value.type() == udVariant::Type::Array)
      return udR_Failure_; // TODO: incorrect type

    switch (pDesc->type)
    {
      case udPropertyType::Boolean:
        return pDesc->setter.set(this, value.asBool());
      case udPropertyType::Integer:
        return pDesc->setter.set(this, value.asInt());
      case udPropertyType::Float:
        return pDesc->setter.set(this, value.asFloat());
      case udPropertyType::String:
        return pDesc->setter.set(this, value.asString());
      case udPropertyType::Component:
        return pDesc->setter.set(this, value.asComponent());
      default:
        return udR_Failure_;
    }
  }

  if (value.type() != udVariant::Type::Array)
    return udR_Failure_; // TODO: incorrect type

  udSlice<udVariant> array = value.asArray();

  // allocate mem on the stack
  size_t count;
  if (pDesc->arrayLength == -1)
    count = array.length;
  else
  {
    if (array.length != pDesc->arrayLength)
      return udR_Failure_; // TODO: proper result for this
    count = pDesc->arrayLength;
  }
  void *pMem = alloca(BytesForArray(pDesc->type, count));

  udResult r = udR_Failure_;
  switch (pDesc->type)
  {
    case udPropertyType::Boolean:
      for (size_t i = 0; i < count; ++i)
        ((bool*)pMem)[i] = array[i].asBool();
      return pDesc->setter.set(this, udSlice<bool>((bool*)pMem, count));
    case udPropertyType::Integer:
      for (size_t i = 0; i < count; ++i)
        ((int64_t*)pMem)[i] = array[i].asInt();
      return pDesc->setter.set(this, udSlice<int64_t>((int64_t*)pMem, count));
    case udPropertyType::Float:
      for (size_t i = 0; i < count; ++i)
        ((double*)pMem)[i] = array[i].asFloat();
      return pDesc->setter.set(this, udSlice<double>((double*)pMem, count));
    case udPropertyType::String:
      for (size_t i = 0; i < count; ++i)
        ((udString*)pMem)[i] = array[i].asString();
      return pDesc->setter.set(this, udSlice<udString>((udString*)pMem, count));
    case udPropertyType::Component:
      for (size_t i = 0; i < count; ++i)
        new((udComponentRef*)pMem + i) udComponentRef(array[i].asComponent());
      r = pDesc->setter.set(this, udSlice<udComponentRef>((udComponentRef*)pMem, count));
      for (size_t i = 0; i < count; ++i)
        ((udComponentRef*)pMem)[i].~udComponentRef();
      break;
    default:
      break;
  }
  return r;
}

udResult udComponent::GetPropertyBool(udString property, bool &result) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->type != udPropertyType::Boolean)
    return udR_Failure_; // TODO: useful message
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength != 0)
    return udR_Failure_; // TODO: return better error message

  return pDesc->getter.get(this, result);
}

udResult udComponent::GetPropertyInt(udString property, int64_t &result) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->type != udPropertyType::Integer)
    return udR_Failure_; // TODO: useful message
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength != 0)
    return udR_Failure_; // TODO: return better error message

  return pDesc->getter.get(this, result);
}

udResult udComponent::GetPropertyIntArray(udString property, udSlice<int64_t> result) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->type != udPropertyType::Integer)
    return udR_Failure_; // TODO: useful message
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength == 0)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength != -1 && result.length < pDesc->arrayLength)
    return udR_Failure_; // TODO: return better error message

   return pDesc->getter.getArray(this, result);
}

udResult udComponent::GetPropertyFloat(udString property, double &result) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->type != udPropertyType::Float)
    return udR_Failure_; // TODO: useful message
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength != 0)
    return udR_Failure_; // TODO: return better error message

  return pDesc->getter.get(this, result);
}

udResult udComponent::GetPropertyFloatArray(udString property, udSlice<double> result) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->type != udPropertyType::Float)
    return udR_Failure_; // TODO: useful message
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength == 0)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength != -1 && result.length < pDesc->arrayLength)
    return udR_Failure_; // TODO: return better error message

  return pDesc->getter.getArray(this, result);
}

udResult udComponent::GetPropertyString(udString property, udString &result) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->type != udPropertyType::String)
    return udR_Failure_; // TODO: useful message
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength != 0)
    return udR_Failure_; // TODO: return better error message

  return pDesc->getter.get(this, result);
}

udResult udComponent::GetPropertyStringArray(udString property, udSlice<udString> result) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->type != udPropertyType::String)
    return udR_Failure_; // TODO: useful message
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength == 0)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength != -1 && result.length < pDesc->arrayLength)
    return udR_Failure_; // TODO: return better error message

  return pDesc->getter.getArray(this, result);
}

udResult udComponent::GetPropertyComponent(udString property, udComponentRef &result) const
{
  const udPropertyDesc *pDesc = FindProperty(property);
  if (!pDesc)
    return udR_Failure_; // TODO: return property doesn't exist error
  if (pDesc->type != udPropertyType::Component)
    return udR_Failure_; // TODO: useful message
  if (pDesc->flags & udPropertyFlags::udPF_NoRead)
    return udR_Failure_; // TODO: return better error message
  if (pDesc->arrayLength != 0)
    return udR_Failure_; // TODO: return better error message

  return pDesc->getter.get(this, result);
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
