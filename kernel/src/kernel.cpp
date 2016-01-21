#include "ep/cpp/platform.h"
#include "ep/cpp/plugin.h"
#include "kernel.h"

#include "hal/hal.h"
#include "components/stream.h"
#include "components/file.h"
#include "components/console.h"
#include "components/memstream.h"
#include "components/lua.h"
#include "components/logger.h"
#include "components/timer.h"
#include "components/uiconsole.h"
#include "components/nodes/geomnode.h"
#include "components/nodes/udnode.h"
#include "components/pluginmanager.h"
#include "components/pluginloader.h"
#include "components/nativepluginloader.h"
#include "components/resourcemanager.h"
#include "components/commandmanager.h"
#include "components/project.h"
#include "ep/cpp/component/resource/buffer.h"
#include "ep/cpp/component/resource/arraybuffer.h"
#include "components/resources/text.h"
#include "components/resources/menu.h"
#include "components/resources/model.h"
#include "components/resources/kvpstore.h"
#include "components/resources/metadata.h"
#include "components/datasources/imagesource.h"
#include "components/datasources/geomsource.h"
#include "components/datasources/uddatasource.h"
#include "components/activities/viewer.h"

// Components that do the Impl dance
#include "components/componentimpl.h"
#include "components/viewimpl.h"
#include "components/uicomponentimpl.h"
#include "components/viewportimpl.h"
#include "components/windowimpl.h"
#include "components/activities/activityimpl.h"
#include "components/resources/resourceimpl.h"
#include "components/resources/udmodelimpl.h"
#include "components/resources/bufferimpl.h"
#include "components/resources/arraybufferimpl.h"
#include "components/resources/materialimpl.h"
#include "components/resources/shaderimpl.h"
#include "components/nodes/nodeimpl.h"
#include "components/nodes/cameraimpl.h"
#include "components/nodes/simplecameraimpl.h"
#include "components/sceneimpl.h"
#include "components/datasources/datasourceimpl.h"

#include "renderscene.h"
#include "eplua.h"
#include "stdcapture.h"

#include "udPlatformUtil.h"

namespace ep {

epResult udRenderScene_Init(Kernel*);
epResult udRenderScene_InitRender(Kernel*);

epResult udRenderScene_Deinit(Kernel*);
epResult udRenderScene_DeinitRender(Kernel*); // Not sure if both Deinit's are necessary

}

namespace kernel {

static ep::Instance *MakeInterface(Kernel *pKernel)
{
  ep::Instance *pInstance = new ep::Instance;

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

epResult Kernel::Create(Kernel **ppInstance, Slice<const KeyValuePair> commandLine, int renderThreadCount)
{
  epResult result;
  StreamRef spDebugFile, spConsole;
  Kernel *pKernel = CreateInstanceInternal(commandLine);

  EP_ERROR_NULL(pKernel, epR_Failure);

  pKernel->pRenderer = new Renderer(pKernel, renderThreadCount);

  // register all the builtin component types
  pKernel->RegisterComponentType<Component, ComponentImpl>();
  pKernel->RegisterComponentType<DataSource, DataSourceImpl>();
  pKernel->RegisterComponentType<Broadcaster>();
  pKernel->RegisterComponentType<Stream>();
  pKernel->RegisterComponentType<File>();
  pKernel->RegisterComponentType<Console>();
  pKernel->RegisterComponentType<MemStream>();
  pKernel->RegisterComponentType<Logger>();
  pKernel->RegisterComponentType<PluginManager>();
  pKernel->RegisterComponentType<PluginLoader>();
  pKernel->RegisterComponentType<NativePluginLoader>();
  pKernel->RegisterComponentType<ResourceManager>();
  pKernel->RegisterComponentType<CommandManager>();
  pKernel->RegisterComponentType<Project>();
  pKernel->RegisterComponentType<Timer>();
  pKernel->RegisterComponentType<Lua>();
  pKernel->RegisterComponentType<UIComponent, UIComponentImpl>();
  pKernel->RegisterComponentType<UIConsole>();
  pKernel->RegisterComponentType<Viewport, ViewportImpl>();
  pKernel->RegisterComponentType<Window, WindowImpl>();
  pKernel->RegisterComponentType<View, ViewImpl>();
  pKernel->RegisterComponentType<Scene, SceneImpl>();

  // resources
  pKernel->RegisterComponentType<Resource, ResourceImpl>();
  pKernel->RegisterComponentType<Buffer, BufferImpl>();
  pKernel->RegisterComponentType<ArrayBuffer, ArrayBufferImpl>();
  pKernel->RegisterComponentType<UDModel, UDModelImpl>();
  pKernel->RegisterComponentType<Shader, ShaderImpl>();
  pKernel->RegisterComponentType<Material, MaterialImpl>();
  pKernel->RegisterComponentType<Model>();
  pKernel->RegisterComponentType<Text>();
  pKernel->RegisterComponentType<Menu>();
  pKernel->RegisterComponentType<KVPStore>();
  pKernel->RegisterComponentType<Metadata>();

  // nodes
  pKernel->RegisterComponentType<Node, NodeImpl>();
  pKernel->RegisterComponentType<Camera, CameraImpl>();
  pKernel->RegisterComponentType<SimpleCamera, SimpleCameraImpl>();
  pKernel->RegisterComponentType<GeomNode>();
  pKernel->RegisterComponentType<UDNode>();

  // data sources
  pKernel->RegisterComponentType<ImageSource>();
  pKernel->RegisterComponentType<GeomSource>();
  pKernel->RegisterComponentType<UDDataSource>();

  // activities
  pKernel->RegisterComponentType<Activity, ActivityImpl>();
  pKernel->RegisterComponentType<Viewer>();

  // init the HAL
  epHAL_Init();

  // create internal stuff
  pKernel->spLua = pKernel->CreateComponent<Lua>();

  // create logger and default streams
  pKernel->spLogger = pKernel->CreateComponent<Logger>();
  pKernel->spLogger->DisableCategory(LogCategories::Trace);

  spDebugFile = pKernel->CreateComponent<File>({ { "path", "epKernel.log" }, { "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });
  if (spDebugFile)
  {
    pKernel->spLogger->AddStream(spDebugFile);
    spDebugFile->WriteLn("\n*** Logging started ***");
  }

  spConsole = pKernel->CreateComponent<Console>({ { "output", ConsoleOutputs::StdDbg } });
  if (spConsole)
     pKernel->spLogger->AddStream(spConsole);

  // resource manager
  pKernel->spResourceManager = pKernel->CreateComponent<ResourceManager>();

  // command manager
  pKernel->spCommandManager = pKernel->CreateComponent<CommandManager>();

  // Init capture and broadcast of stdout/stderr
  pKernel->spStdOutBC = pKernel->CreateComponent<Broadcaster>();
  pKernel->stdOutCapture = new StdCapture(stdout);
  pKernel->spStdErrBC = pKernel->CreateComponent<Broadcaster>();
  pKernel->stdErrCapture = new StdCapture(stderr);

  // platform init
  EP_ERROR_CHECK(pKernel->InitInternal());

epilogue:
  if (result != epR_Success)
  {
    // TODO: clean up code
  }
  *ppInstance = pKernel;
  return result;
}

epResult Kernel::DoInit(Kernel *pKernel)
{
  // init the components
  if (pKernel->InitComponents() != epR_Success)
  {
    EPASSERT(false, "Oh no! Can't boot!");
    return epR_Failure;
  }

  epResult result = udRenderScene_Init(pKernel);
  if (result != epR_Success)
    return result;

  result = udRenderScene_InitRender(pKernel);

  if (result != epR_Success)
    return result;

  // prepare the plugins
  pKernel->spPluginManager = pKernel->CreateComponent<PluginManager>();
  if (!pKernel->spPluginManager)
    return epR_Failure;

  PluginLoaderRef spNativePluginLoader = pKernel->CreateComponent<NativePluginLoader>();
  if (!spNativePluginLoader)
    return epR_Failure;
  pKernel->spPluginManager->RegisterPluginLoader(spNativePluginLoader);

  LoadPlugins();

  // make the kernel timers
  pKernel->spStreamerTimer = pKernel->CreateComponent<Timer>({ { "duration", 33 }, { "timertype", "Interval" } });
  if (!pKernel->spStreamerTimer)
    return epR_Failure;
  pKernel->spStreamerTimer->Elapsed.Subscribe(FastDelegate<void()>(pKernel, &Kernel::StreamerUpdate));

  pKernel->spUpdateTimer = pKernel->CreateComponent<Timer>({ { "duration", 16 }, { "timertype", "Interval" } });
  if (!pKernel->spUpdateTimer)
    return epR_Failure;
  pKernel->spUpdateTimer->Elapsed.Subscribe(FastDelegate<void()>(pKernel, &Kernel::Update));

  // call application init
  return SendMessage("$init", "#", "init", nullptr);
}

void Kernel::LoadPlugins()
{
  Slice<String> pluginFilenames;

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

epResult Kernel::Destroy()
{
  // call application deinit
  SendMessage("$deinit", "#", "deinit", nullptr);

  spUpdateTimer = nullptr;
  spStreamerTimer = nullptr;
  spPluginManager = nullptr;

  epResult result = epR_Success;
  epResult renderSceneRenderResult = udRenderScene_DeinitRender(this);
  epResult renderSceneResult = udRenderScene_Deinit(this);

  udOctree_Shutdown();

  delete stdOutCapture;
  delete stdErrCapture;

  spStdErrBC = nullptr;
  spStdOutBC = nullptr;

  spCommandManager = nullptr;
  spResourceManager = nullptr;
  spLogger = nullptr;
  spLua = nullptr;

  delete pRenderer;

  epResult halResult = epHAL_Deinit();

  delete this;

  if (renderSceneRenderResult != epR_Success)
    result = renderSceneRenderResult;
  else if (renderSceneResult != epR_Success)
    result = renderSceneResult;
  else if (halResult != epR_Success)
    result = halResult;

  return result;
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

epResult Kernel::SendMessage(String target, String sender, String message, const Variant &data)
{
  if (target.empty())
    return epR_Failure; // TODO: no target!!

  char targetType = target.popFront();
  if (targetType == '@')
  {
    // component message
    Component **ppComponent = instanceRegistry.Get(target);
    if (ppComponent)
    {
      ComponentRef spComponent(*ppComponent);
      return spComponent->ReceiveMessage(message, sender, data);
    }
    else
    {
      // TODO: check if it's in the foreign component registry and send it there

      return epR_Failure; // TODO: no component!
    }
  }
  else if (targetType == '#')
  {
    // kernel message
    if (target.eq(uid))
    {
      // it's for me!
      return ReceiveMessage(sender, message, data);
    }
    else
    {
      // TODO: foreign kernels?!

      return epR_Failure; // TODO: invalid kernel!
    }
  }
  else if (targetType == '$')
  {
    // registered message
    MessageCallback *pHandler = messageHandlers.Get(target);
    if (pHandler)
    {
      pHandler->callback(sender, message, data);
      return epR_Success;
    }
    else
      return epR_Failure; // TODO: no message handler
  }

  return epR_Failure; // TODO: error, invalid target!
}

epResult Kernel::ReceiveMessage(String sender, String message, const Variant &data)
{

  return epR_Success;
}

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
  kernel::ComponentDesc *pDesc = new kernel::ComponentDesc(desc);

  // add to registry
  componentRegistry.Insert(desc.info.id, ComponentType{ pDesc, 0 });

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

epResult Kernel::CreateComponent(String typeId, Variant::VarMap initParams, ep::ComponentRef *pNewInstance)
{
  if (!pNewInstance)
    return epR_Failure;

  ComponentType *pType = componentRegistry.Get(typeId);
  if (!pType)
    return epR_Failure;

  try
  {
    const ep::ComponentDesc *pDesc = pType->pDesc;

    // TODO: should we have a better uid generator than this?
    MutableString64 newUid(Concat, pDesc->info.id, pType->createCount++);

    // attempt to create an instance
    ep::ComponentRef spComponent(pDesc->pCreateInstance(pDesc, this, newUid, initParams));
    if (!spComponent)
      return epR_AllocFailure;

    // post-create init
    spComponent->Init(initParams);

    // add to the component registry
    instanceRegistry.Insert(spComponent->uid, spComponent.ptr());
    if (spLua)
      spLua->SetGlobal(spComponent->uid, spComponent.ptr());

    // TODO: inform partner kernels that I created a component
    //...

    *pNewInstance = spComponent;
    return epR_Success;
  }
  catch (EPException &e)
  {
    LogDebug(3, "Create component failed: {0}", String(e.pError->message));
    return e.pError->error;
  }
  catch (...)
  {
    LogDebug(3, "Create component failed!");
    return epR_Failure;
  }
}

epResult Kernel::DestroyComponent(Component *pInstance)
{
  if (spLua && spLua.ptr() != pInstance)
    spLua->SetGlobal(pInstance->uid, nullptr);

  if (pInstance->name)
    namedInstanceRegistry.Remove(pInstance->name);
  instanceRegistry.Remove(pInstance->uid);

  // TODO: inform partners that I destroyed a component
  //...

  return epR_Success;
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

epResult Kernel::InitComponents()
{
  epResult r = epR_Success;
  for (auto i : componentRegistry)
  {
    if (i.value.pDesc->pInit)
    {
      r = i.value.pDesc->pInit(this);
      if (r != epR_Success)
        break;
    }
  }
  return r;
}

epResult Kernel::InitRender()
{
  epHAL_InitRender();

  return epR_Success;
}

epResult Kernel::DeinitRender()
{
  epHAL_DeinitRender();

  return epR_Success;
}

void Kernel::Exec(String code)
{
  spLua->Execute(code);
}

epResult Kernel::RegisterExtensions(const ep::ComponentDesc *pDesc, const Slice<const String> exts)
{
  for (const String &e : exts)
  {
    extensionsRegistry.Insert(e, pDesc);
  }

  return epR_Success;
}

DataSourceRef Kernel::CreateDataSourceFromExtension(String ext, Variant::VarMap initParams)
{
  const ep::ComponentDesc **pDesc = extensionsRegistry.Get(ext);
  if (!pDesc)
    return nullptr;

  ep::ComponentRef spNewDataSource = nullptr;
  epResult r = CreateComponent((*pDesc)->info.id, initParams, &spNewDataSource);
  if (r != epR_Success)
    return nullptr;

  return shared_pointer_cast<DataSource>(spNewDataSource);
}

} // namespace kernel

// synchronised pointer destroy function (it's here because there's no udsharedptr.cpp file)
template<class T>
void SynchronisedPtr<T>::destroy()
{
  struct S
  {
    void Destroy(Kernel *pKernel)
    {
      ((SharedPtr<T>&)this)->release();
    }
  };

  pKernel->DispatchToMainThread(MakeDelegate((S*)this, &S::Destroy));
  pInstance = nullptr;
}



