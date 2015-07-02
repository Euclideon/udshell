
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
    udPF_NoRead, // flags
    udPropertyDisplayType::Default // displayType
  }
};
const udComponentDesc udComponent::descriptor =
{
  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  udComponentType::Component, // type

  "component", // id
  nullptr,     // parentId
  "udComponent",    // displayName
  "Is a component", // description

  nullptr, // pInit
  nullptr, // pInitRender
  nullptr, // pCreateInstance

  props,                           // pProperties
  sizeof(props) / sizeof(props[0]) // numProperties
};


udResult udComponent::ReceiveMessage(udString message, udString sender, udString data)
{
  if(message.eqi("set"))
  {
    const char *pProp = nullptr, *pVal = nullptr;

    UDASSERT(false, "TODO: parse 'prop = val'");

    SetProperty(pProp, pVal);
  }
  else if(message.eqi("get"))
  {
    if(!sender.empty())
    {
      char mem[1024];
      udSlice<char> buffer(mem, sizeof(mem));
      GetProperty(data, &buffer);
      SendMessage(sender, "val", buffer);
    }
  }

  return udR_Success;
}

udResult udComponent::SendMessage(udString target, udString message, udString data)
{
  return pKernel->SendMessage(target, uid, message, data);
}

udResult udComponent::SetProperty(udString property, int64_t value)
{
  char buffer[24];
  sprintf(buffer, "%lld", value);
  SetProperty(property, buffer);
  return udR_Success;
}
udResult udComponent::GetProperty(udString property, int64_t *pValue)
{
  char mem[24];
  udSlice<char> buffer(mem, sizeof(mem));
  GetProperty(property, &buffer);
  int64_t i = property.parseInt();
  if(pValue)
    *pValue = i;
  return udR_Success;
}

// float overload
udResult udComponent::SetProperty(udString property, double value)
{
  char buffer[64];
  sprintf(buffer, "%lg", value);
  SetProperty(property, buffer);
  return udR_Success;
}

udResult udComponent::GetProperty(udString property, double *pValue)
{
  char mem[64];
  udSlice<char> buffer(mem, sizeof(mem));
  GetProperty(property, &buffer);
  double f = property.parseFloat();
  if(pValue)
    *pValue = f;
  return udR_Success;
}

udResult udComponent::SetProperty(udString property, udComponent *pObject)
{
  SetProperty(property, pObject->uid);
  return udR_Success;
}

udResult udComponent::GetProperty(udString property, udComponent **ppObject)
{
  char mem[64], mem2[64];
  udSlice<char> buffer(mem, sizeof(mem));
  GetProperty(property, &buffer);
  udComponent **ppComponent = pKernel->instanceRegistry.Get(udString(buffer).toStringz(mem2, 64));
  if(!ppComponent)
    return udR_Failure_; // TODO: better error type... (object not found!)
  if(ppObject)
    *ppObject = *ppComponent;
  return udR_Success;
}
