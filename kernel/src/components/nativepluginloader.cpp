#include "components/nativepluginloader.h"
#include "kernel.h"

#include "ep/c/plugin.h"
#include "ep/c/internal/kernel_inl.h"
#include "ep/c/internal/component_inl.h"

extern "C" {
  typedef bool (epPlugin_InitProc)(epPluginInstance *pPlugin);
}

namespace ep {

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

  static Variant shimFunc(const Getter * const _pGetter, const Component *pThis)
  {
    PluginGetter *pGetter = (PluginGetter*)_pGetter;
    Variant v;
    (epVariant&)v = pGetter->pGetter((const epComponent*)pThis, pThis->GetUserData());
    return v;
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

  static void shimFunc(const Setter * const _pSetter, Component *pThis, const Variant &value)
  {
    PluginSetter *pSetter = (PluginSetter*)_pSetter;
    return pSetter->pSetter((epComponent*)pThis, pThis->GetUserData(), (const epVariant*)&value);
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

  static Variant shimFunc(const Method* const _pMethod, Component *pThis, Slice<Variant> args)
  {
    PluginMethod *pMethod = (PluginMethod*)_pMethod;
    Variant v;
    (epVariant&)v = pMethod->pMethod((epComponent*)pThis, pThis->GetUserData(), (const epVariant*)args.ptr, args.length);
    return v;
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

  static Variant shimFunc(const StaticFunc* const _pFunc, Slice<Variant> args)
  {
    PluginStaticFunc *pFunc = (PluginStaticFunc*)_pFunc;
    Variant v;
    (epVariant&)v = pFunc->pFunc((const epVariant*)args.ptr, args.length);
    return v;
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

  static void doSubscribe(const VarEvent *_pEv, const ComponentRef &c, const Variant::VarDelegate &d)
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
static Component* CreatePluginInstance(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
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
    return pKernel->SendMessage(target, sender, message, *(Variant*)pData);
  },

  // RegisterComponentType
  [](epKernel *_pKernel, const epComponentDesc *_pDesc) -> epResult
  {
    Kernel *pKernel = (Kernel*)_pKernel;

    const ComponentDesc *pSuper = pKernel->GetComponentDesc(MutableString64(Concat, _pDesc->baseClass, "plugin"));

    // TODO: sizeof(...) needs to be dynamic, we need to ask it from pSuper somehow...
    if (_pDesc->pOverrides->structSize != sizeof(epComponentOverrides))
    {
      return epR_Failure;
    }

    ComponentDesc *pDesc = new ComponentDesc(
      (ComponentDesc*)pSuper, // pSuperDesc
      EPSHELL_APIVERSION,     // epVersion
      _pDesc->info.pluginVersion,  // pluginVersion
      _pDesc->info.id,          // id
      _pDesc->info.displayName, // displayName
      _pDesc->info.description, // description
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
  [](epKernel *_pKernel, epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance) -> epResult
  {
    Kernel *pKernel = (Kernel*)_pKernel;

    ComponentRef spC;
    epResult r = pKernel->CreateComponent(typeId, Slice<const KeyValuePair>((const KeyValuePair*)pInitParams, numInitParams), &spC);

    if (r != epR_Success)
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

    pPluginInstance->Alloc = [](size_t size) -> void*
    {
      return epAlloc(size);
    },
    pPluginInstance->AllocAligned = [](size_t size, size_t alignment) -> void*
    {
      return epAllocAligned(size, alignment, epAF_None);
    },
    pPluginInstance->Free = [](void *pMem) -> void
    {
      epFree(pMem);
    },

    pPluginInstance->AssertFailed = [](epString condition, epString message, epString file, int line) -> void
    {
#if EPASSERT_ON
      epAssertFailed(condition, message, file, line);
#endif
    },

    pPluginInstance->DestroyComponent = [](epComponent *pInstance) -> void
    {
      // NOTE: this was called when an RC reached zero...
      Component *pC = (Component*)pInstance;
      pC->DecRef(); //       and then dec it with the internal function which actually performs the cleanup
    };

    pPluginInstance->pKernelAPI = &s_kernelAPI;
    pPluginInstance->pComponentAPI = &g_componentAPI;
  }
  return pPluginInstance;
}

// ----- Everything we need! -----


ComponentDesc NativePluginLoader::descriptor =
{
  &PluginLoader::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "nativepluginloader", // id
  "Native Plugin Loader", // displayName
  "Loads native plugins", // description

  //Slice<CPropertyDesc>(props, EPARRAYSIZE(props)), // properties
  //Slice<CMethodDesc>(methods, EPARRAYSIZE(methods)), // methods
  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

Slice<const String> NativePluginLoader::GetSupportedExtensions() const
{
#if defined(EP_WINDOWS)
  static Array<const String> s_ext = { ".dll" };
#elif defined(EP_OSX) || defined(EP_IPHONE)
  static Array<const String> s_ext = { ".dylib" };
#else
  static Array<const String> s_ext = { ".so" };
#endif
  return s_ext;
}

bool NativePluginLoader::LoadPlugin(String filename)
{
#if defined(EP_WINDOWS)
  // Convert UTF-8 to UTF-16 -- TODO use UD helper functions or add some to hal?
  int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename.ptr, (int)filename.length, nullptr, 0);
  wchar_t *widePath = (wchar_t*)alloca(sizeof(wchar_t) * (len + 1));
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename.ptr, (int)filename.length, widePath, len) == 0)
    return false;
  widePath[len] = 0;

  // try and load library
  HMODULE hDll = LoadLibraryW(widePath);
  if (hDll == NULL)
    return false;
#if defined(EP_ARCH_X86)
  const char *pFuncName = "_epPlugin_Init";
#else
  const char *pFuncName = "epPlugin_Init";
#endif
  epPlugin_InitProc *pInit = (epPlugin_InitProc*)GetProcAddress(hDll, pFuncName);
  if (!pInit)
  {
    FreeLibrary(hDll);
    return false;
  }
#else
  epPlugin_InitProc *pInit = nullptr;

  EPASSERT(false, "Not yet supported!");

  if (!pInit)
    return false;
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
