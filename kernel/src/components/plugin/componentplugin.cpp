
#include "components/plugin/componentplugin.h"

namespace ep {

// kernel API definition
epComponentAPI g_componentAPI =
{
  // GetUID
  [](epComponent *pComponent) -> epString
  {
    Component *pC = (Component*)pComponent;
    return pC->uid;
  },
  // GetName
  [](epComponent *pComponent) -> epString
  {
    Component *pC = (Component*)pComponent;
    return pC->name;
  },

  // IsType
  [](epComponent *pComponent, epString type) -> bool
  {
    Component *pC = (Component*)pComponent;
    return pC->IsType(type);
  },

  //GetProperty
  [](epComponent *pComponent, epString property) -> epVariant
  {
    Component *pC = (Component*)pComponent;
    return pC->GetProperty(property);
  },
  // SetProperty
  [](epComponent *pComponent, epString property, const epVariant *pValue) -> void
  {
    Component *pC = (Component*)pComponent;
    pC->SetProperty(property, *pValue);
  },

  // CallMethod
  [](epComponent *pComponent, epString method, const epVariant *pArgs, size_t numArgs) -> epVariant
  {
    Component *pC = (Component*)pComponent;
    return pC->CallMethod(method, epSlice<const epVariant>(pArgs, numArgs));
  },

  // Subscribe
  [](epComponent *pComponent, epString eventName, const epVarDelegate *pDelegate) -> void
  {
    Component *pC = (Component*)pComponent;
    pC->Subscribe(eventName, (epVariant::VarDelegate&)pDelegate);
  },

  // SendMessage
  [](epComponent *pComponent, epString target, epString message, const epVariant *pData) -> epResult
  {
    Component *pC = (Component*)pComponent;
    return pC->SendMessage(target, message, *pData);
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

//  epSlice<CPropertyDesc>(props, UDARRAYSIZE(props)) // propeties
};



} // namespace ep
