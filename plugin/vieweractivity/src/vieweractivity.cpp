#include "ep/c/plugin.h"
#include "ep/c/componentdesc.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/event.h"

#include "ep/cpp/internal/i/icomponent.h"

using namespace ep;

#define PLUGIN_VER 100

// C component
/* TODO: this has to be reworked since we switched the API to C++...
static epComponentDesc s_desc;

struct TestComponent
{
  epComponent *pBase;

  int x;
};

static epComponentOverrides s_testComponentVirtuals = {
  sizeof(epComponentOverrides),

  // pCreateInstance
  [](epComponent *pBaseInstance, const epVarMap *epUnusedParam(pInitParams)) -> void*
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
  [](epComponent *epUnusedParam(pBaseInstance), void *epUnusedParam(pDerivedInstance)) -> void
  {
  },

  // pReceiveMessage
  [](epComponent *epUnusedParam(pBaseInstance), void *epUnusedParam(pDerivedInstance), epString epUnusedParam(message), epString epUnusedParam(sender), const epVariant *epUnusedParam(pData)) -> void
  {
  }
};

static epPropertyDesc s_prop = {
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
*/

// C++ component

class TestComponent2 : public Component
{
  EP_DECLARE_COMPONENT(TestComponent2, Component, 100, "Plugin Test!")
public:

  int GetX() const { return x; }
  void SetX(int _x) { x = _x; }

  int MethodX(int _x) { Kernel::GetInstance()->LogWarning(0, "Wow {0} {1}", _x, x); return x; }

  static int FuncX(int _x) { Kernel::GetInstance()->LogWarning(0, "Wow {0}", _x); return _x; }

  Event<> EvX;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(X, "Desc...", "Display", 0),
    };
  }

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(MethodX, "Wow!"),
    };
  }

  static Array<const EventInfo> GetEvents()
  {
    return{
      EP_MAKE_EVENT(EvX, "Wow!"),
    };
  }

  static Array<const StaticFuncInfo> GetStaticFuncs()
  {
    return{
      EP_MAKE_STATICFUNC(FuncX, "Wow!"),
    };
  }

protected:
  TestComponent2(const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams)
    : Component(_pType, _pKernel, _uid, initParams) {}

  int x;
};

extern "C" bool epPluginAttach()
{
/*
  // C test
  s_desc.info.pluginVersion = PLUGIN_VER;
  s_desc.info.id = "plugtest";          // an id for this component
  s_desc.info.displayName = "plugin Test"; // display name
  s_desc.info.description = "Plugin Test!"; // description
  s_desc.baseClass = "component";
  s_desc.pOverrides = &s_testComponentVirtuals;
  s_desc.pProperties = &s_prop;
  s_desc.numProperties = 1;
  s_desc.pMethods = nullptr;
  s_desc.numMethods = 0;
  s_desc.pEvents = nullptr;
  s_desc.numEvents = 0;
  s_desc.pStaticFuncs = nullptr;
  s_desc.numStaticFuncs = 0;

  epKernel_RegisterComponentType(&s_desc);
*/
  // C++ test
  Kernel::GetInstance()->RegisterComponentType<TestComponent2>();

  return true;
}
