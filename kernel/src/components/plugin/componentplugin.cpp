
#include "components/plugin/componentplugin.h"

namespace ep {

// kernel API definition
epComponentAPI g_componentAPI =
{
  // GetUID
  [](const epComponent *pComponent) -> epString
  {
    Component *pC = (Component*)pComponent;
    return (epString&)pC->uid;
  },
  // GetName
  [](const epComponent *pComponent) -> epString
  {
    Component *pC = (Component*)pComponent;
    return (epString&)pC->name;
  },

  // IsType
  [](const epComponent *pComponent, epString type) -> bool
  {
    Component *pC = (Component*)pComponent;
    return pC->IsType(type);
  },

  // GetProperty
  [](const epComponent *pComponent, epString property) -> epVariant
  {
    Component *pC = (Component*)pComponent;
    epVariant r;
    new(&r) Variant(pC->GetProperty(property));
    return r;
  },
  // SetProperty
  [](epComponent *pComponent, epString property, const epVariant *pValue) -> void
  {
    Component *pC = (Component*)pComponent;
    pC->SetProperty(property, *(const Variant*)pValue);
  },

  // CallMethod
  [](epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs) -> epVariant
  {
    Component *pC = (Component*)pComponent;
    epVariant r;
    new(&r) Variant(pC->CallMethod(method, Slice<const Variant>((const Variant*)pArgs, numArgs)));
    return r;
  },

  // Subscribe
  [](epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate) -> void
  {
    Component *pC = (Component*)pComponent;
    pC->Subscribe(eventName, (Variant::VarDelegate&)pDelegate);
  },

  // SendMessage
  [](epComponent *pComponent, epString target, epString message, const epVariant *pData) -> epResult
  {
    Component *pC = (Component*)pComponent;
    return pC->SendMessage(target, message, *(const Variant*)pData);
  }
};


ComponentDesc ComponentPlugin::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "componentplugin", // id
  "ComponentBase", // displayName
  "Component plugin base class", // description

//  Slice<CPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};



} // namespace ep
