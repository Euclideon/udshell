#include "components/pluginmanager.h"
#include "kernel.h"

#include "ep/epplugin.h"
#include "ep/epcomponent.h"

extern "C" {
  typedef bool (epPlugin_Init)(epPluginInstance *pPlugin);
}

namespace ep
{

// ----- plugin glue -----
class PluginGetter : public Getter
{
public:
  PluginGetter(epGetter *pGetter) : Getter(nullptr), pGetter(pGetter)
  {
    shim = &shimFunc;
  }

protected:
  epGetter *pGetter;

  static epVariant shimFunc(const Getter * const _pGetter, const ep::Component *pThis)
  {
    PluginGetter *pGetter = (PluginGetter*)_pGetter;
    return pGetter->pGetter((const epComponent*)pThis, pThis->GetUserData());
  }
};

class PluginSetter : public Setter
{
public:
  PluginSetter(epSetter *pSetter) : Setter(nullptr), pSetter(pSetter)
  {
    shim = &shimFunc;
  }

protected:
  epSetter *pSetter;

  static void shimFunc(const Setter * const _pSetter, ep::Component *pThis, const epVariant &value)
  {
    PluginSetter *pSetter = (PluginSetter*)_pSetter;
    return pSetter->pSetter((epComponent*)pThis, pThis->GetUserData(), &value);
  }
};

class PluginMethod : public Method
{
public:
  PluginMethod(epMethodCall *pMethod) : Method(nullptr), pMethod(pMethod)
  {
    shim = &shimFunc;
  }

protected:
  epMethodCall *pMethod;

  static epVariant shimFunc(const Method* const _pMethod, ep::Component *pThis, epSlice<epVariant> args)
  {
    PluginMethod *pMethod = (PluginMethod*)_pMethod;
    return pMethod->pMethod((epComponent*)pThis, pThis->GetUserData(), args.ptr, args.length);
  }
};

class PluginStaticFunc : public StaticFunc
{
public:
  PluginStaticFunc(epStaticCall *pFunc) : StaticFunc(nullptr), pFunc(pFunc)
  {
    shim = &shimFunc;
  }

protected:
  epStaticCall *pFunc;

  static epVariant shimFunc(const StaticFunc* const _pFunc, epSlice<epVariant> args)
  {
    PluginStaticFunc *pFunc = (PluginStaticFunc*)_pFunc;
    return pFunc->pFunc(args.ptr, args.length);
  }
};

class PluginEvent : public VarEvent
{
public:
  PluginEvent(epSubscribe *pSubscribeFunc) : VarEvent(nullptr), pSubscribeFunc(pSubscribeFunc)
  {
    pSubscribe = &doSubscribe;
  }

protected:
  epSubscribe *pSubscribeFunc;

  static void doSubscribe(const VarEvent *_pEv, const ComponentRef &c, const epVariant::VarDelegate &d)
  {
    PluginEvent *pEv = (PluginEvent*)_pEv;
    pEv->pSubscribeFunc((epComponent*)c.ptr(), c->GetUserData(), (epVarDelegate*&)d);
  }
};

// ----- plugin registration helpers -----
// TODO: all these functions result in memory leaks!!!
PropertyDesc* MakePluginPropertyDesc(const epPropertyDesc &prop)
{
  return new PropertyDesc(
    PropertyInfo(
      prop.id,
      prop.displayName,
      prop.description,
      prop.displayType,
      prop.flags
    ),
    prop.pGetter ? new PluginGetter(prop.pGetter) : nullptr,
    prop.pSetter ? new PluginSetter(prop.pSetter) : nullptr
  );
}
MethodDesc* MakePluginMethodDesc(const epMethodDesc &method)
{
  return new MethodDesc(
    FunctionInfo(
      method.id,
      method.description
    ),
    method.pCall ? new PluginMethod(method.pCall) : nullptr
  );
}
StaticFuncDesc* MakePluginStaticFuncDesc(const epStaticFuncDesc &func)
{
  return new StaticFuncDesc(
    FunctionInfo(
      func.id,
      func.description
    ),
    func.pCall ? new PluginStaticFunc(func.pCall) : nullptr
  );
}
EventDesc* MakePluginEventDesc(const epEventDesc &ev)
{
  return new EventDesc(
    EventInfo(
      ev.id,
      ev.displayName,
      ev.description
    ),
    ev.pSubscribe ? new PluginEvent(ev.pSubscribe) : nullptr
  );
}

// ----- public plugin API definition -----
static Component* CreatePluginInstance(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
{
  return pType->pSuperDesc->pCreateInstance(pType, pKernel, uid, initParams);
}

// kernel API definition
static epKernelAPI s_kernelAPI =
{
  // SendMessage
  [](epKernel *_pKernel, epString target, epString sender, epString message, const epVariant* pData) -> epResult
  {
    Kernel *pKernel = (Kernel*)_pKernel;
    return pKernel->SendMessage(target, sender, message, *pData);
  },

  // RegisterComponentType
  [](epKernel *_pKernel, const epComponentDesc *_pDesc) -> epResult
  {
    Kernel *pKernel = (Kernel*)_pKernel;

    epMutableString64 baseName; baseName.concat(_pDesc->baseClass, "plugin");
    const ComponentDesc *pSuper = pKernel->GetComponentDesc(baseName);

    // TODO: sizeof(...) needs to be dynamic, we need to ask it from pSuper somehow...
    if (_pDesc->pOverrides->structSize != sizeof(epComponentOverrides))
    {
      return udR_Failure_;
    }

    ComponentDesc *pDesc = new ComponentDesc(
      (ComponentDesc*)pSuper, // pSuperDesc
      EPSHELL_APIVERSION,     // epVersion
      _pDesc->pluginVersion,  // pluginVersion
      _pDesc->id,          // id
      _pDesc->displayName, // displayName
      _pDesc->description, // description
      nullptr, // properties
      nullptr, // methods
      nullptr, // events
      nullptr, // staticFuncs
      nullptr, // pInit (not used by plugins)
      &CreatePluginInstance, // pCreateInstance
      _pDesc // pExternalDesc
      );

    return pKernel->RegisterComponentType(pDesc);
  },

  // CreateComponent
  [](epKernel *_pKernel, epString typeId, epInitParams initParams, epComponent **ppNewInstance) -> epResult
  {
    Kernel *pKernel = (Kernel*)_pKernel;

    ComponentRef spC;
    udResult r = pKernel->CreateComponent(typeId, initParams, &spC);

    if (r != udR_Success)
      return r;

    spC->IncRef();
    *ppNewInstance = (epComponent*)spC.ptr();
    return r;
  },

  // FindComponent
  [](epKernel *_pKernel, epString uid) -> epComponent*
  {
    Kernel *pKernel = (Kernel*)_pKernel;

    ComponentRef spC = pKernel->FindComponent(uid);
    spC->IncRef();
    return (epComponent*)spC.ptr();
  },

  // Exec
  [](epKernel *_pKernel, epString code) -> void
  {
    Kernel *pKernel = (Kernel*)_pKernel;
    pKernel->Exec(code);
  },

  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr
};

extern epComponentAPI g_componentAPI;

// HAX: Kernel::GetPluginInterface() defined here to save a whole bunch of extern's and header pollution
epPluginInstance *Kernel::GetPluginInterface()
{
  if (!pPluginInstance)
  {
    pPluginInstance = new epPluginInstance;
    pPluginInstance->apiVersion = EPSHELL_APIVERSION;
    pPluginInstance->pKernelInstance = (epKernel*)this;
    pPluginInstance->DestroyComponent = [](epComponent *pInstance) -> void
    {
      Component *pC = (Component*)pInstance;
      pC->IncRef();
      pC->DecRef();
    };
    pPluginInstance->pKernelAPI = &s_kernelAPI;
    pPluginInstance->pComponentAPI = &g_componentAPI;
  }
  return pPluginInstance;
}

// ----- Everything we need! -----


/*
static CMethodDesc methods[] =
{

};
static CPropertyDesc props[] =
{

};
*/

ComponentDesc PluginManager::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "pluginmanager", // id
  "Plugin Manager", // displayName
  "Manages plugins", // description

  //epSlice<CPropertyDesc>(props, EPARRAYSIZE(props)), // properties
  //epSlice<CMethodDesc>(methods, EPARRAYSIZE(methods)), // methods
  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

PluginManager::PluginManager(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{

}

PluginManager::~PluginManager()
{

}

bool PluginManager::LoadPlugin(epString filename)
{
#if defined(EP_WINDOWS)
  // Convert UTF-8 to UTF-16 -- TODO use UD helper functions or add some to hal?
  int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename.ptr, (int)filename.length, nullptr, 0);
  wchar_t *widePath = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename.ptr, (int)filename.length, widePath, len) == 0)
    *widePath = 0;
  widePath[len] = 0;

  // try and load library
  HMODULE hDll = LoadLibraryW(widePath);
  if (hDll == NULL)
    return false;

  epPlugin_Init *pInit = (epPlugin_Init*)GetProcAddress(hDll, "epPlugin_Init");
  if (!pInit)
  {
    FreeLibrary(hDll);
    return false;
  }
#else
  EPASSERT(false, "Not yet supported!");
#endif

  bool bSuccess = pInit(pKernel->GetPluginInterface());

  if (!bSuccess)
  {
#if defined(EP_WINDOWS)
    FreeLibrary(hDll);
#endif
  }

  // TODO: add plugin to plugin registry
  // TODO: plugin component types need to be associated with the plugin, so when the plugin unloads, the component types can be removed

  return bSuccess;
}

} // namespace ep
