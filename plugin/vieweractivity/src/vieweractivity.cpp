#include "ep/c/plugin.h"
#include "ep/c/componentdesc.h"
#include "ep/c/component.h"
#include "ep/c/kernel.h"

#define PLUGIN_VER 100

epComponentDesc desc;

struct TestComponent
{
  epComponent *pBase;

  int x;
};

epComponentOverrides s_testComponentVirtuals = {
  sizeof(epComponentOverrides),

  // pCreateInstance
  [](epComponent *pBaseInstance, const epKeyValuePair *epUnusedParam(pInitParams), size_t epUnusedParam(numInitParams)) -> void*
  {
    TestComponent *pNew = new TestComponent;
    pNew->pBase = pBaseInstance;
    pNew->x = 100;
    return pNew;
  },

  // pDestroy
  [](epComponent *epUnusedParam(pBaseInstance), void *pDerivedInstance) -> void
  {
    TestComponent *pC = (TestComponent*)pDerivedInstance;
    delete pC;
  },

  // pInitComplete
  [](epComponent *epUnusedParam(pBaseInstance), void *epUnusedParam(pDerivedInstance)) -> epResult
  {
    return epR_Success;
  },

  // pReceiveMessage
  [](epComponent *epUnusedParam(pBaseInstance), void *epUnusedParam(pDerivedInstance), epString epUnusedParam(message), epString epUnusedParam(sender), const epVariant *epUnusedParam(pData)) -> epResult
  {
    return epR_Success;
  }
};

epPropertyDesc prop = {
  "prop",
  "Prop",
  "Test Prop",

  "type",
  0,

  [](const epComponent *epUnusedParam(pBaseComponent), const void *pDerivedInstance) -> epVariant
  {
    TestComponent *pC = (TestComponent*)pDerivedInstance;
    return epVariant_CreateInt(pC->x);
  },
  [](epComponent *epUnusedParam(pBaseComponent), void *pDerivedInstance, const epVariant *pValue) -> void
  {
    TestComponent *pC = (TestComponent*)pDerivedInstance;
    pC->x = (int)epVariant_AsInt(*pValue);
  }
};

extern "C" bool epPluginAttach()
{
  desc.pluginVersion = PLUGIN_VER;
  desc.id = "plugtest";          // an id for this component
  desc.displayName = "plugin Test"; // display name
  desc.description = "Plugin Test!"; // description
  desc.baseClass = "component";
  desc.pOverrides = &s_testComponentVirtuals;
  desc.pProperties = &prop;
  desc.numProperties = 1;
  desc.pMethods = nullptr;
  desc.numMethods = 0;
  desc.pEvents = nullptr;
  desc.numEvents = 0;
  desc.pStaticFuncs = nullptr;
  desc.numStaticFuncs = 0;

  epKernel_RegisterComponentType(&desc);

  return true;
}
