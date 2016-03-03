#include "ep/cpp/platform.h"
#include "ep/cpp/plugin.h"

#include "kernelimpl.h"

#include "hal/hal.h"
#include "components/file.h"
#include "components/console.h"
#include "components/memstream.h"
#include "components/lua.h"
#include "components/logger.h"
#include "components/timer.h"
#include "components/uiconsole.h"
#include "components/nodes/geomnode.h"
#include "components/pluginmanager.h"
#include "components/pluginloader.h"
#include "components/nativepluginloader.h"
#include "components/project.h"
#include "components/resources/text.h"
#include "components/resources/kvpstore.h"
#include "components/resources/metadata.h"
#include "components/datasources/imagesource.h"
#include "components/datasources/geomsource.h"
#include "components/datasources/uddatasource.h"

// Components that do the Impl dance
#include "components/componentimpl.h"
#include "components/viewimpl.h"
#include "components/uicomponentimpl.h"
#include "components/viewportimpl.h"
#include "components/windowimpl.h"
#include "components/commandmanagerimpl.h"
#include "components/resourcemanagerimpl.h"
#include "components/activityimpl.h"
#include "components/resources/resourceimpl.h"
#include "components/resources/udmodelimpl.h"
#include "components/resources/bufferimpl.h"
#include "components/resources/arraybufferimpl.h"
#include "components/resources/materialimpl.h"
#include "components/resources/shaderimpl.h"
#include "components/resources/menuimpl.h"
#include "components/resources/modelimpl.h"
#include "components/nodes/nodeimpl.h"
#include "components/nodes/udnodeimpl.h"
#include "components/nodes/cameraimpl.h"
#include "components/nodes/simplecameraimpl.h"
#include "components/sceneimpl.h"
#include "components/datasources/datasourceimpl.h"
#include "components/broadcasterimpl.h"
#include "components/streamimpl.h"

#include "renderscene.h"
#include "eplua.h"
#include "stdcapture.h"

#include "udPlatformUtil.h"

namespace ep {

static ep::Instance *MakeInterface(Kernel *pKernel)
{
  ep::Instance *pInstance = epNew ep::Instance;

  pInstance->apiVersion = EP_APIVERSION;

  pInstance->pKernelInstance = pKernel;

  pInstance->Alloc = [](size_t size) -> void*
  {
    return epAlloc(size);
  },
  pInstance->AllocAligned = [](size_t size, size_t alignment) -> void*
  {
    return epAllocAligned(size, alignment, epAF_None);
  },
  pInstance->Free = [](void *pMem) -> void
  {
    epFree(pMem);
  },

  pInstance->AssertFailed = [](String condition, String message, String file, int line) -> void
  {
#if EPASSERT_ON
    epAssertFailed(condition, message, file, line);
#endif
  },

  pInstance->DestroyComponent = [](Component *pInstance) -> void
  {
    // NOTE: this was called when an RC reached zero...
    pInstance->DecRef(); // dec it with the internal function which actually performs the cleanup
  };

  return pInstance;
}

static ComponentDesc *MakeKernelDescriptor(ComponentDesc *pType)
{
  ComponentDesc *pDesc = epNew ComponentDesc;
  EPTHROW_IF_NULL(pDesc, epR_AllocFailure, "Memory allocation failed");

  pDesc->info = Kernel::MakeDescriptor();
  pDesc->baseClass = Component::ComponentID();

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pSuperDesc = nullptr;

//  pDesc->properties = CreateHelper<ComponentType>::GetProperties();
//  pDesc->methods = CreateHelper<ComponentType>::GetMethods();
//  pDesc->events = CreateHelper<ComponentType>::GetEvents();
//  pDesc->staticFuncs = CreateHelper<ComponentType>::GetStaticFuncs();

  if (pType)
  {
    pType->pSuperDesc = pDesc;
    pDesc = pType;
  }
  return pDesc;
}
Kernel::Kernel(ComponentDesc *_pType, Variant::VarMap commandLine)
  : Component(MakeKernelDescriptor(_pType), nullptr, "kernel0", commandLine)
{
  // alloc impl
  pImpl = UniquePtr<Impl>(epNew KernelImpl(this, commandLine));
  GetImpl()->StartInit(commandLine);
}

Kernel::~Kernel()
{
  // HACK: undo chicken/egg hacks
  Component::pImpl = nullptr;
  (Kernel*&)pKernel = nullptr;

  // HACK: destroy the descriptor we fabricated...
  const ComponentDesc *pKernelDesc = pType;
  (const ComponentDesc*&)pType = pType->pSuperDesc;
  epDelete pKernelDesc;
}

Kernel* Kernel::CreateInstance(Variant::VarMap commandLine, int renderThreadCount)
{
  if (!commandLine.Get("renderThreadCount"))
    commandLine.Insert("renderThreadCount", renderThreadCount);

  return CreateInstanceInternal(commandLine);
}


KernelImpl::KernelImpl(Kernel *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
  , componentRegistry(256)
  , instanceRegistry(8192)
  , namedInstanceRegistry(4096)
  , foreignInstanceRegistry(4096)
  , messageHandlers(64)
{
  ep::s_pInstance = MakeInterface(pInstance);
}

void KernelImpl::StartInit(Variant::VarMap initParams)
{
  // init the kernel
  epscope(fail) { epDebugFormat("Error creating Kernel\n"); };

  renderThreadCount = initParams["renderThreadCount"].as<int>();

  // register the base Component type
  pInstance->RegisterComponentType<Component, ComponentImpl>();

  // HACK: update the descriptor with the base class (bootup chicken/egg)
  ComponentDesc *pDesc = const_cast<ComponentDesc*>(pInstance->pType);
  while (pDesc->pSuperDesc)
    pDesc = const_cast<ComponentDesc*>(pDesc->pSuperDesc);
  pDesc->pSuperDesc = componentRegistry.Get(Component::ComponentID())->pDesc;

  // HACK: fix up the base class since we have a kernel instance (bootup chicken/egg)
  (Kernel*&)pInstance->pKernel = pInstance;
  pInstance->Component::pImpl = pInstance->Component::CreateImpl(initParams);

  // register all the builtin component types
  pInstance->RegisterComponentType<DataSource, DataSourceImpl>();
  pInstance->RegisterComponentType<Broadcaster, BroadcasterImpl>();
  pInstance->RegisterComponentType<Stream, StreamImpl>();
  pInstance->RegisterComponentType<File>();
  pInstance->RegisterComponentType<Console>();
  pInstance->RegisterComponentType<MemStream>();
  pInstance->RegisterComponentType<Logger>();
  pInstance->RegisterComponentType<PluginManager>();
  pInstance->RegisterComponentType<PluginLoader>();
  pInstance->RegisterComponentType<NativePluginLoader>();
  pInstance->RegisterComponentType<ResourceManager, ResourceManagerImpl>();
  pInstance->RegisterComponentType<CommandManager, CommandManagerImpl>();
  pInstance->RegisterComponentType<Project>();
  pInstance->RegisterComponentType<Timer>();
  pInstance->RegisterComponentType<Lua>();
  pInstance->RegisterComponentType<UIComponent, UIComponentImpl>();
  pInstance->RegisterComponentType<UIConsole>();
  pInstance->RegisterComponentType<Viewport, ViewportImpl>();
  pInstance->RegisterComponentType<Window, WindowImpl>();
  pInstance->RegisterComponentType<View, ViewImpl>();
  pInstance->RegisterComponentType<Scene, SceneImpl>();
  pInstance->RegisterComponentType<Activity, ActivityImpl>();

  // resources
  pInstance->RegisterComponentType<Resource, ResourceImpl>();
  pInstance->RegisterComponentType<Buffer, BufferImpl>();
  pInstance->RegisterComponentType<ArrayBuffer, ArrayBufferImpl>();
  pInstance->RegisterComponentType<UDModel, UDModelImpl>();
  pInstance->RegisterComponentType<Shader, ShaderImpl>();
  pInstance->RegisterComponentType<Material, MaterialImpl>();
  pInstance->RegisterComponentType<Model, ModelImpl>();
  pInstance->RegisterComponentType<Text>();
  pInstance->RegisterComponentType<Menu, MenuImpl>();
  pInstance->RegisterComponentType<KVPStore>();
  pInstance->RegisterComponentType<Metadata>();

  // nodes
  pInstance->RegisterComponentType<Node, NodeImpl>();
  pInstance->RegisterComponentType<Camera, CameraImpl>();
  pInstance->RegisterComponentType<SimpleCamera, SimpleCameraImpl>();
  pInstance->RegisterComponentType<GeomNode>();
  pInstance->RegisterComponentType<UDNode, UDNodeImpl>();

  // data sources
  pInstance->RegisterComponentType<ImageSource>();
  pInstance->RegisterComponentType<GeomSource>();
  pInstance->RegisterComponentType<UDDataSource>();

  // init the HAL
  EPTHROW_EPRESULT(epHAL_Init(), "epHAL_Init() failed");

  // create logger and default streams
  spLogger = pInstance->CreateComponent<Logger>();
  spLogger->DisableCategory(LogCategories::Trace);

  StreamRef spDebugFile = pInstance->CreateComponent<File>({ { "name", "logfile" }, { "path", "epKernel.log" }, { "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });
  spLogger->AddStream(spDebugFile);
  spDebugFile->WriteLn("\n*** Logging started ***");

#if EP_DEBUG
  StreamRef spConsole = pInstance->CreateComponent<Console>({ { "output", ConsoleOutputs::StdDbg }, {"name", "debugout"} });
  spLogger->AddStream(spConsole);
#endif

  // resource manager
  spResourceManager = pInstance->CreateComponent<ResourceManager>({ { "name", "resourcemanager" } });

  // command manager
  spCommandManager = pInstance->CreateComponent<CommandManager>({ { "name", "commandmanager" } });

  // Init capture and broadcast of stdout/stderr
  spStdOutBC = pInstance->CreateComponent<Broadcaster>({ { "name", "stdoutbc" } });
  stdOutCapture = epNew StdCapture(stdout);
  epscope(fail) { epDelete stdOutCapture; };
  spStdErrBC = pInstance->CreateComponent<Broadcaster>({ { "name", "stderrbc" } });
  stdErrCapture = epNew StdCapture(stderr);
  epscope(fail) { epDelete stdErrCapture; };

  // create lua VM
  spLua = pInstance->CreateComponent<Lua>();

  bKernelCreated = true; // TODO: remove this?
}

void KernelImpl::FinishInit()
{
  // create the renderer
  spRenderer = SharedPtr<Renderer>::create(pInstance, renderThreadCount);

  // init the components
  InitComponents();

  // prepare the plugins
  spPluginManager = pInstance->CreateComponent<PluginManager>({ { "name", "pluginmanager" } });

  PluginLoaderRef spNativePluginLoader = pInstance->CreateComponent<NativePluginLoader>();
  spPluginManager->RegisterPluginLoader(spNativePluginLoader);

  LoadPlugins();

  // make the kernel timers
  spStreamerTimer = pInstance->CreateComponent<Timer>({ { "duration", 33 },{ "timertype", "Interval" } });
  spStreamerTimer->Elapsed.Subscribe(FastDelegate<void()>(this, &KernelImpl::StreamerUpdate));

  spUpdateTimer = pInstance->CreateComponent<Timer>({ { "duration", 16 },{ "timertype", "Interval" } });
  spUpdateTimer->Elapsed.Subscribe(FastDelegate<void()>(this, &KernelImpl::Update));

  // call application init
  return SendMessage("$init", "#", "init", nullptr);
}

KernelImpl::~KernelImpl()
{
  spLua = nullptr;

  spResourceManager = nullptr;
  spCommandManager = nullptr;

  epDelete stdOutCapture;
  epDelete stdErrCapture;
  stdOutCapture = nullptr;
  stdErrCapture = nullptr;

  spStdErrBC = nullptr;
  spStdOutBC = nullptr;

  spLogger = nullptr;

  if (instanceRegistry.begin() != instanceRegistry.end())
  {
    int count = 0;
    epDebugFormat("!!!WARNING: Some Components have not been freed\n");

    for (const auto &c : instanceRegistry)
    {
      ++count;
      epDebugFormat("Unfreed Component: {0} ({1}) refCount {2} \n", c.key, c.value->GetName(), c.value->RefCount());
    }
    epDebugFormat("{0} Unfreed Component(s)\n", count);
  }

  epHAL_Deinit();

  for (const auto &c : componentRegistry)
    epDelete c.value.pDesc;

  epDelete(ep::s_pInstance);
}

void KernelImpl::Shutdown()
{
  // TODO: Consider whether or not to catch exceptions and then continuing the deinit path or just do nothing.

  spStreamerTimer->Elapsed.Unsubscribe(FastDelegate<void()>(this, &KernelImpl::StreamerUpdate));
  spUpdateTimer->Elapsed.Unsubscribe(FastDelegate<void()>(this, &KernelImpl::Update));

  // call application deinit
  SendMessage("$deinit", "#", "deinit", nullptr);

  SetFocusView(nullptr);

  spUpdateTimer = nullptr;
  spStreamerTimer = nullptr;

  spPluginManager = nullptr;

  spRenderer = nullptr;
}

void KernelImpl::LoadPlugins()
{
  Array<String> pluginFilenames;

  // TODO: scan nominated plugin folder and build list of all plugins

  size_t numRemaining = pluginFilenames.length;
  size_t lastTry;
  do
  {
    // since plugins may depend on other plugins, we'll keep trying to reload plugins while loads are succeeding
    lastTry = numRemaining;
    for (auto &filename : pluginFilenames)
    {
      if (!filename)
        continue;
      if (spPluginManager->LoadPlugin(filename))
      {
        filename = nullptr;
        --numRemaining;
      }
    }
  } while (numRemaining && numRemaining < lastTry);
}

void KernelImpl::Update()
{
  static uint64_t last = udPerfCounterStart();
  uint64_t now = udPerfCounterStart();
  double sec = (double)udPerfCounterMilliseconds(last, now) / 1000.0;
  last = now;

  RelayStdIO();

  pInstance->UpdatePulse.Signal(sec);
}

void KernelImpl::RelayStdIO()
{
  if (stdOutCapture)
  {
    String str = stdOutCapture->GetCapture();
    if (!str.empty())
      spStdOutBC->Write(str);
  }
  if (stdErrCapture)
  {
    String str = stdErrCapture->GetCapture();
    if (!str.empty())
      spStdErrBC->Write(str);
  }
}

void KernelImpl::StreamerUpdate()
{
  udStreamerStatus streamerStatus = { 0 };
  udOctree_Update(&streamerStatus);

  // TODO: Find a cleaner way of doing this.  We have to keep rendering while the streamer is active.
  // We need more than just a global active , we need an active per view.
  if (streamerStatus.active)
  {
    SceneRef spScene = spFocusView->GetScene();
    if (spScene)
      spScene->MakeDirty();
  }
}

Array<const ep::ComponentDesc *> KernelImpl::GetDerivedComponentDescs(String id, bool bIncludeBase)
{
  ComponentType *compType = componentRegistry.Get(id);
  if (compType)
    return GetDerivedComponentDescs(compType->pDesc, bIncludeBase);
  else
    return nullptr;
}

Array<const ep::ComponentDesc *> KernelImpl::GetDerivedComponentDescs(const ep::ComponentDesc *pBase, bool bIncludeBase)
{
  Array<const ep::ComponentDesc *> derivedDescs;

  for (auto ct : componentRegistry)
  {
    const ep::ComponentDesc *pDesc = ct.value.pDesc;
    if(!bIncludeBase)
      pDesc = pDesc->pSuperDesc;

    while (pDesc)
    {
      if (pDesc == pBase)
      {
        derivedDescs.concat(ct.value.pDesc);
        break;
      }
      pDesc = pDesc->pSuperDesc;
    }
  }

  return derivedDescs;
}

void KernelImpl::SendMessage(String target, String sender, String message, const Variant &data)
{
  EPASSERT_THROW(!target.empty(), epR_InvalidArgument, "target was empty");

  char targetType = target.popFront();
  if (targetType == '@')
  {
    // component message
    Component **ppComponent = instanceRegistry.Get(target);
    if (ppComponent)
    {
      ComponentRef spComponent(*ppComponent);
      size_t errorDepth = ErrorLevel();
      try
      {
        spComponent->ReceiveMessage(message, sender, data);
        if (ErrorLevel() > errorDepth)
        {
          LogError("Message Handler {0} failed {1}", target, GetError()->message);
          PopErrorToLevel(errorDepth);
        }
      }
      catch (std::exception &e)
      {
        LogError("Message Handler {0} failed: {1}", target, e.what());
        PopErrorToLevel(errorDepth);
      }
      catch (...)
      {
        LogError("Message Handler {0} failed", target);
        PopErrorToLevel(errorDepth);
      }
    }
    else
    {
      // TODO: check if it's in the foreign component registry and send it there
      EPTHROW_ERROR(epR_Failure, "Target component not found");
    }
  }
  else if (targetType == '#')
  {
    // kernel message
    if (target.eq(uid))
    {
      // it's for me!
      size_t errorDepth = ErrorLevel();
      try
      {
        ReceiveMessage(sender, message, data);
        if (ErrorLevel() > errorDepth)
        {
          LogError("Message Handler {0} failed {1}", target, GetError()->message);
          PopErrorToLevel(errorDepth);
        }
      }
      catch (std::exception &e)
      {
        LogError("Message Handler {0} failed: {1}", target, e.what());
        PopErrorToLevel(errorDepth);
      }
      catch (...)
      {
        LogError("Message Handler {0} failed", target);
        PopErrorToLevel(errorDepth);
      }
    }
    else
    {
      // TODO: foreign kernels?!
      EPTHROW_ERROR(epR_Failure, "Invalid Kernel");
    }
  }
  else if (targetType == '$')
  {
    // registered message
    MessageCallback *pHandler = messageHandlers.Get(target);
    if (pHandler)
    {
      size_t errorDepth = ErrorLevel();
      try
      {
        pHandler->callback(sender, message, data);
        if (ErrorLevel() > errorDepth)
        {
          LogError("Message Handler {0} failed {1}", target, GetError()->message);
          PopErrorToLevel(errorDepth);
        }
      }
      catch (std::exception &e)
      {
        LogError("Message Handler {0} failed: {1}", target, e.what());
        PopErrorToLevel(errorDepth);
      }
      catch (...)
      {
        LogError("Message Handler {0} failed", target);
        PopErrorToLevel(errorDepth);
      }
    }
    else
    {
      EPTHROW_ERROR(epR_Failure, "No Message Handler");
    }
  }
  else
  {
    EPTHROW_ERROR(epR_Failure, "Invalid target");
  }
}

// ---------------------------------------------------------------------------------------
void KernelImpl::DispatchToMainThread(MainThreadCallback callback)
{
  EPASSERT(false, "!!shouldn't be here!!");
}

// ---------------------------------------------------------------------------------------
void KernelImpl::DispatchToMainThreadAndWait(MainThreadCallback callback)
{
  EPASSERT(false, "!!shouldn't be here!!");
}

// TODO: Take this hack out once RecieveMessage's body is implemented
#if defined(EP_COMPILER_VISUALC) && defined(EP_RELEASE)
#pragma optimize("", off)
#endif // defined(EP_COMPILER_VISUALC) && defined(EP_RELEASE)
void KernelImpl::ReceiveMessage(String sender, String message, const Variant &data)
{

}
#if defined(EP_COMPILER_VISUALC) && defined(EP_RELEASE)
#pragma optimize("", on)
#endif // defined(EP_COMPILER_VISUALC) && defined(EP_RELEASE)

void KernelImpl::RegisterMessageHandler(SharedString _name, MessageHandler messageHandler)
{
  messageHandlers.Insert(_name, MessageCallback{ _name, messageHandler });
}

const ep::ComponentDesc* KernelImpl::RegisterComponentType(const ep::ComponentDesc &desc)
{
  if (desc.info.id.exists('@') || desc.info.id.exists('$') || desc.info.id.exists('#'))
  {
    EPASSERT(false, "Invalid component id");
    return nullptr;
  }

  // create the descriptor
  kernel::ComponentDesc *pDesc = epNew kernel::ComponentDesc(desc);

  // add to registry
  componentRegistry.Insert(desc.info.id, ComponentType{ pDesc, 0 });

  if (bKernelCreated && pDesc->pInit)
    pDesc->pInit(pInstance);

  return pDesc;
}

void* KernelImpl::CreateImpl(String componentType, Component *_pInstance, Variant::VarMap initParams)
{
  kernel::ComponentDesc *pDesc = (kernel::ComponentDesc*)GetComponentDesc(componentType);
  if (pDesc->pCreateImpl)
    return pDesc->pCreateImpl(_pInstance, initParams);
  return nullptr;
}

const ep::ComponentDesc* KernelImpl::GetComponentDesc(String id)
{
  ComponentType *pCT = componentRegistry.Get(id);
  if (!pCT)
    return nullptr;
  return pCT->pDesc;
}

ep::ComponentRef KernelImpl::CreateComponent(String typeId, Variant::VarMap initParams)
{
  ComponentType *_pType = componentRegistry.Get(typeId);
  EPASSERT_THROW(_pType, epR_InvalidArgument, "typeId failed to lookup ComponentType");

  try
  {
    const ep::ComponentDesc *pDesc = _pType->pDesc;

    // TODO: should we have a better uid generator than this?
    MutableString64 newUid(Concat, pDesc->info.id, _pType->createCount++);

    // attempt to create an instance
    ep::ComponentRef spComponent(pDesc->pCreateInstance(pDesc, pInstance, newUid, initParams));

    // post-create init
    spComponent->Init(initParams);

    // add to the component registry
    instanceRegistry.Insert(spComponent->uid, spComponent.ptr());

    // TODO: inform partner kernels that I created a component
    //...

    return std::move(spComponent);
  }
  catch (std::exception &e)
  {
    LogWarning(3, "Create component failed: {0}", String(e.what()));
    throw;
  }
  catch (...)
  {
    LogWarning(3, "Create component failed!");
    throw;
  }
}

void KernelImpl::DestroyComponent(Component *_pInstance)
{
  if (_pInstance->name)
    namedInstanceRegistry.Remove(_pInstance->name);
  instanceRegistry.Remove(_pInstance->uid);

  // TODO: inform partners that I destroyed a component
  //...
}

ep::ComponentRef KernelImpl::FindComponent(String _name) const
{
  if (_name.empty() || _name[0] == '$' || _name[0] == '#')
    return nullptr;
  if (_name[0] == '@')
    _name.popFront();
  Component * const * ppComponent = namedInstanceRegistry.Get(_name);
  if (!ppComponent)
    ppComponent = instanceRegistry.Get(_name);
  return ppComponent ? ep::ComponentRef(*ppComponent) : nullptr;
}

void KernelImpl::InitComponents()
{
  for (auto i : componentRegistry)
  {
    if (i.value.pDesc->pInit)
      i.value.pDesc->pInit(pInstance);
  }
}

void KernelImpl::InitRender()
{
  epResult result = epHAL_InitRender();
  EPASSERT_THROW(result == epR_Success, result, "epHAL_InitRender() Failed");
}

void KernelImpl::DeinitRender()
{
  epHAL_DeinitRender();
}

void KernelImpl::Exec(String code)
{
  spLua->Execute(code);
}

void KernelImpl::Log(int kind, int level, String text, String component) const
{
  if (spLogger)
    spLogger->Log(level, text, (LogCategories)kind, component);
}

const AVLTree<String, const ep::ComponentDesc *> &KernelImpl::GetExtensionsRegistry() const
{
  return extensionsRegistry;
}

void KernelImpl::RegisterExtensions(const ep::ComponentDesc *pDesc, const Slice<const String> exts)
{
  for (const String &e : exts)
    extensionsRegistry.Insert(e, pDesc);
}

DataSourceRef KernelImpl::CreateDataSourceFromExtension(String ext, Variant::VarMap initParams)
{
  const ep::ComponentDesc **ppDesc = extensionsRegistry.Get(ext);
  EPASSERT_THROW(ppDesc, epR_Failure, "No datasource for extension {0}", ext);

  return component_cast<DataSource>(CreateComponent((*ppDesc)->info.id, initParams));
}

SharedPtr<Renderer> KernelImpl::GetRenderer() const
{
  return spRenderer;
}

CommandManagerRef KernelImpl::GetCommandManager() const
{
  return spCommandManager;
}

ResourceManagerRef KernelImpl::GetResourceManager() const
{
  return spResourceManager;
}

BroadcasterRef KernelImpl::GetStdOutBroadcaster() const
{
  return spStdOutBC;
}

BroadcasterRef KernelImpl::GetStdErrBroadcaster() const
{
  return spStdErrBC;
}

ViewRef KernelImpl::GetFocusView() const
{
  return spFocusView;
}

ViewRef KernelImpl::SetFocusView(ViewRef spView)
{
  ViewRef spOld = spFocusView;
  spFocusView = spView;
  return spOld;
}

} // namespace ep
