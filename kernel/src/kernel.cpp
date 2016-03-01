#include "ep/cpp/platform.h"
#include "ep/cpp/plugin.h"
#include "kernel.h"

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

namespace kernel {

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


Kernel::Kernel()
  : componentRegistry(256)
  , instanceRegistry(8192)
  , namedInstanceRegistry(4096)
  , foreignInstanceRegistry(4096)
  , messageHandlers(64)
{
  ep::s_pInstance = MakeInterface(this);
}

Kernel::~Kernel()
{
  spRenderer = nullptr;
  epDelete(ep::s_pInstance);

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

  for (const auto &c : componentRegistry)
    epDelete c.value.pDesc;
}

void Kernel::Create(Kernel **ppInstance, Slice<const KeyValuePair> commandLine, int _renderThreadCount)
{
  epscope(fail) { epDebugFormat("Error creating Kernel\n"); };

  StreamRef spDebugFile, spConsole;
  Kernel *pKernel = CreateInstanceInternal(commandLine);

  EPASSERT_THROW(pKernel, epR_Failure, "CreateInstanceInternal() Failed");

  pKernel->renderThreadCount = _renderThreadCount;

  // register all the builtin component types
  pKernel->RegisterComponentType<Component, ComponentImpl>();
  pKernel->RegisterComponentType<DataSource, DataSourceImpl>();
  pKernel->RegisterComponentType<Broadcaster, BroadcasterImpl>();
  pKernel->RegisterComponentType<Stream, StreamImpl>();
  pKernel->RegisterComponentType<File>();
  pKernel->RegisterComponentType<Console>();
  pKernel->RegisterComponentType<MemStream>();
  pKernel->RegisterComponentType<Logger>();
  pKernel->RegisterComponentType<PluginManager>();
  pKernel->RegisterComponentType<PluginLoader>();
  pKernel->RegisterComponentType<NativePluginLoader>();
  pKernel->RegisterComponentType<ResourceManager, ResourceManagerImpl>();
  pKernel->RegisterComponentType<CommandManager, CommandManagerImpl>();
  pKernel->RegisterComponentType<Project>();
  pKernel->RegisterComponentType<Timer>();
  pKernel->RegisterComponentType<Lua>();
  pKernel->RegisterComponentType<UIComponent, UIComponentImpl>();
  pKernel->RegisterComponentType<UIConsole>();
  pKernel->RegisterComponentType<Viewport, ViewportImpl>();
  pKernel->RegisterComponentType<Window, WindowImpl>();
  pKernel->RegisterComponentType<View, ViewImpl>();
  pKernel->RegisterComponentType<Scene, SceneImpl>();
  pKernel->RegisterComponentType<Activity, ActivityImpl>();

  // resources
  pKernel->RegisterComponentType<Resource, ResourceImpl>();
  pKernel->RegisterComponentType<Buffer, BufferImpl>();
  pKernel->RegisterComponentType<ArrayBuffer, ArrayBufferImpl>();
  pKernel->RegisterComponentType<UDModel, UDModelImpl>();
  pKernel->RegisterComponentType<Shader, ShaderImpl>();
  pKernel->RegisterComponentType<Material, MaterialImpl>();
  pKernel->RegisterComponentType<Model, ModelImpl>();
  pKernel->RegisterComponentType<Text>();
  pKernel->RegisterComponentType<Menu, MenuImpl>();
  pKernel->RegisterComponentType<KVPStore>();
  pKernel->RegisterComponentType<Metadata>();

  // nodes
  pKernel->RegisterComponentType<Node, NodeImpl>();
  pKernel->RegisterComponentType<Camera, CameraImpl>();
  pKernel->RegisterComponentType<SimpleCamera, SimpleCameraImpl>();
  pKernel->RegisterComponentType<GeomNode>();
  pKernel->RegisterComponentType<UDNode, UDNodeImpl>();

  // data sources
  pKernel->RegisterComponentType<ImageSource>();
  pKernel->RegisterComponentType<GeomSource>();
  pKernel->RegisterComponentType<UDDataSource>();

  // init the HAL
  EPTHROW_IF(epHAL_Init() != epR_Success, epR_Failure, "epHAL_Init() failed");

  // create internal stuff
  pKernel->spLua = pKernel->CreateComponent<Lua>();

  // create logger and default streams
  pKernel->spLogger = pKernel->CreateComponent<Logger>();
  pKernel->spLogger->DisableCategory(LogCategories::Trace);

  spDebugFile = pKernel->CreateComponent<File>({ { "name", "logfile" },{ "path", "epKernel.log" },{ "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });
  pKernel->spLogger->AddStream(spDebugFile);
  spDebugFile->WriteLn("\n*** Logging started ***");

#if defined(EP_WINDOWS) && defined(EP_DEBUG_OUTPUT)
  if (IsDebuggerPresent())
  {
    spConsole = pKernel->CreateComponent<Console>({ { "output", ConsoleOutputs::StdDbg }, {"name", "debugout"} });
    pKernel->spLogger->AddStream(spConsole);
  }
#endif

  // resource manager
  pKernel->spResourceManager = pKernel->CreateComponent<ResourceManager>({ {"name", "resourcemanager"} });

  // command manager
  pKernel->spCommandManager = pKernel->CreateComponent<CommandManager>({ {"name", "commandmanager"} });

  // Init capture and broadcast of stdout/stderr
  pKernel->spStdOutBC = pKernel->CreateComponent<Broadcaster>({ {"name", "stdoutbc"} });
  pKernel->stdOutCapture = epNew StdCapture(stdout);
  epscope(fail) { epDelete pKernel->stdOutCapture; };
  pKernel->spStdErrBC = pKernel->CreateComponent<Broadcaster>({ {"name", "stderrbc"} });
  pKernel->stdErrCapture = epNew StdCapture(stderr);
  epscope(fail) { epDelete pKernel->stdErrCapture; };

  // platform init
  pKernel->InitInternal();

  *ppInstance = pKernel;

  pKernel->bKernelCreated = true;
}

void Kernel::DoInit(Kernel *pKernel)
{
  // create the renderer
  pKernel->spRenderer = SharedPtr<Renderer>::create(pKernel, renderThreadCount);

  // init the components
  pKernel->InitComponents();

  // prepare the plugins
  pKernel->spPluginManager = pKernel->CreateComponent<PluginManager>({ {"name", "pluginmanager"} });

  PluginLoaderRef spNativePluginLoader = pKernel->CreateComponent<NativePluginLoader>();
  pKernel->spPluginManager->RegisterPluginLoader(spNativePluginLoader);

  LoadPlugins();

  // make the kernel timers
  pKernel->spStreamerTimer = pKernel->CreateComponent<Timer>({ { "duration", 33 }, { "timertype", "Interval" } });
  pKernel->spStreamerTimer->Elapsed.Subscribe(FastDelegate<void()>(pKernel, &Kernel::StreamerUpdate));

  pKernel->spUpdateTimer = pKernel->CreateComponent<Timer>({ { "duration", 16 }, { "timertype", "Interval" } });
  pKernel->spUpdateTimer->Elapsed.Subscribe(FastDelegate<void()>(pKernel, &Kernel::Update));

  // call application init
  return SendMessage("$init", "#", "init", nullptr);
}

void Kernel::LoadPlugins()
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

void Kernel::Destroy()
{
  // TODO: Consider whether or not to catch exceptions and then continuing the deinit path or just do nothing.

  spStreamerTimer->Elapsed.Unsubscribe(FastDelegate<void()>(this, &Kernel::StreamerUpdate));
  spUpdateTimer->Elapsed.Unsubscribe(FastDelegate<void()>(this, &Kernel::Update));

  // call application deinit
  SendMessage("$deinit", "#", "deinit", nullptr);

  SetFocusView(nullptr);

  spUpdateTimer = nullptr;
  spStreamerTimer = nullptr;
  spPluginManager = nullptr;
  spResourceManager = nullptr;

  epDelete stdOutCapture;
  epDelete stdErrCapture;
  stdOutCapture = nullptr;
  stdErrCapture = nullptr;

  spStdErrBC = nullptr;
  spStdOutBC = nullptr;

  spCommandManager = nullptr;
  spLogger = nullptr;

  // TODO: **this is not okay** the lua VM won't do a final GC until this moment
  //       Lua may have references to render resources (or other stuff), but the renderer is already destroyed!!
  spLua = nullptr;

  epDelete this;

  epHAL_Deinit();
}

void Kernel::Update()
{
  static uint64_t last = udPerfCounterStart();
  uint64_t now = udPerfCounterStart();
  double sec = (double)udPerfCounterMilliseconds(last, now) / 1000.0;
  last = now;

  RelayStdIO();

  UpdatePulse.Signal(sec);
}

void Kernel::RelayStdIO()
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

void Kernel::StreamerUpdate()
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

Array<const ep::ComponentDesc *> Kernel::GetDerivedComponentDescs(const ep::ComponentDesc *pBase, bool bIncludeBase)
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

void Kernel::SendMessage(String target, String sender, String message, const Variant &data)
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

// TODO: Take this hack out once RecieveMessage's body is implemented
#if defined(EP_COMPILER_VISUALC) && defined(EP_RELEASE)
#pragma optimize("", off)
#endif // defined(EP_COMPILER_VISUALC) && defined(EP_RELEASE)
void Kernel::ReceiveMessage(String sender, String message, const Variant &data)
{

}
#if defined(EP_COMPILER_VISUALC) && defined(EP_RELEASE)
#pragma optimize("", on)
#endif // defined(EP_COMPILER_VISUALC) && defined(EP_RELEASE)

void Kernel::RegisterMessageHandler(SharedString name, MessageHandler messageHandler)
{
  messageHandlers.Insert(name, MessageCallback{ name, messageHandler });
}

const ep::ComponentDesc* Kernel::RegisterComponentType(const ep::ComponentDesc &desc)
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
    pDesc->pInit(this);

  return pDesc;
}

void* Kernel::CreateImpl(String componentType, Component *pInstance, Variant::VarMap initParams)
{
  kernel::ComponentDesc *pDesc = (kernel::ComponentDesc*)GetComponentDesc(componentType);
  if (pDesc->pCreateImpl)
    return pDesc->pCreateImpl(pInstance, initParams);
  return nullptr;
}

const ep::ComponentDesc* Kernel::GetComponentDesc(String id)
{
  ComponentType *pCT = componentRegistry.Get(id);
  if (!pCT)
    return nullptr;
  return pCT->pDesc;
}

ep::ComponentRef Kernel::CreateComponent(String typeId, Variant::VarMap initParams)
{
  ComponentType *pType = componentRegistry.Get(typeId);
  EPASSERT_THROW(pType, epR_InvalidArgument, "typeId failed to lookup ComponentType");

  try
  {
    const ep::ComponentDesc *pDesc = pType->pDesc;

    // TODO: should we have a better uid generator than this?
    MutableString64 newUid(Concat, pDesc->info.id, pType->createCount++);

    // attempt to create an instance
    ep::ComponentRef spComponent(pDesc->pCreateInstance(pDesc, this, newUid, initParams));

    // post-create init
    spComponent->Init(initParams);

    // add to the component registry
    instanceRegistry.Insert(spComponent->uid, spComponent.ptr());
    if (spLua)
      spLua->SetGlobal(spComponent->uid, spComponent.ptr());

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

void Kernel::DestroyComponent(Component *pInstance)
{
  if (spLua && spLua.ptr() != pInstance)
    spLua->SetGlobal(pInstance->uid, nullptr);

  if (pInstance->name)
    namedInstanceRegistry.Remove(pInstance->name);
  instanceRegistry.Remove(pInstance->uid);

  // TODO: inform partners that I destroyed a component
  //...
}

ep::ComponentRef Kernel::FindComponent(String name) const
{
  if (name.empty() || name[0] == '$' || name[0] == '#')
    return nullptr;
  if (name[0] == '@')
    name.popFront();
  Component * const * ppComponent = namedInstanceRegistry.Get(name);
  if (!ppComponent)
    ppComponent = instanceRegistry.Get(name);
  return ppComponent ? ep::ComponentRef(*ppComponent) : nullptr;
}

void Kernel::InitComponents()
{
  for (auto i : componentRegistry)
  {
    if (i.value.pDesc->pInit)
      i.value.pDesc->pInit(this);
  }
}

void Kernel::InitRender()
{
  epResult result = epHAL_InitRender();
  EPASSERT_THROW(result == epR_Success, result, "epHAL_InitRender() Failed");
}

void Kernel::DeinitRender()
{
  epHAL_DeinitRender();
}

void Kernel::Exec(String code)
{
  spLua->Execute(code);
}

const AVLTree<String, const ep::ComponentDesc *> &Kernel::GetExtensionsRegistry() const
{
  return extensionsRegistry;
}

void Kernel::RegisterExtensions(const ep::ComponentDesc *pDesc, const Slice<const String> exts)
{
  for (const String &e : exts)
    extensionsRegistry.Insert(e, pDesc);
}

DataSourceRef Kernel::CreateDataSourceFromExtension(String ext, Variant::VarMap initParams)
{
  const ep::ComponentDesc **ppDesc = extensionsRegistry.Get(ext);
  EPASSERT_THROW(ppDesc, epR_Failure, "No datasource for extension {0}", ext);

  return component_cast<DataSource>(CreateComponent((*ppDesc)->info.id, initParams));
}

SharedPtr<Renderer> Kernel::GetRenderer() const
{
  return spRenderer;
}

CommandManagerRef Kernel::GetCommandManager() const
{
  return spCommandManager;
}

ResourceManagerRef Kernel::GetResourceManager() const
{
  return spResourceManager;
}

BroadcasterRef Kernel::GetStdOutBroadcaster() const
{
  return spStdOutBC;
}

BroadcasterRef Kernel::GetStdErrBroadcaster() const
{
  return spStdErrBC;
}

ViewRef Kernel::GetFocusView() const
{
  return spFocusView;
}



} // namespace kernel
