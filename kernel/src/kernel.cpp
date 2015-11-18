#include "ep/cpp/platform.h"
#include "kernel.h"

#include "hal/hal.h"
#include "components/stream.h"
#include "components/file.h"
#include "components/console.h"
#include "components/memstream.h"
#include "components/lua.h"
#include "components/logger.h"
#include "components/timer.h"
#include "components/scene.h"
#include "components/view.h"
#include "components/ui.h"
#include "components/uiconsole.h"
#include "components/viewport.h"
#include "components/window.h"
#include "components/datasource.h"
#include "components/nodes/node.h"
#include "components/nodes/camera.h"
#include "components/nodes/geomnode.h"
#include "components/nodes/udnode.h"
#include "components/pluginmanager.h"
#include "components/pluginloader.h"
#include "components/nativepluginloader.h"
#include "components/resourcemanager.h"
#include "components/shortcutmanager.h"
#include "components/project.h"
#include "components/resources/resource.h"
#include "components/resources/buffer.h"
#include "components/resources/array.h"
#include "components/resources/text.h"
#include "components/resources/shader.h"
#include "components/resources/material.h"
#include "components/resources/model.h"
#include "components/resources/udmodel.h"
#include "components/resources/kvpstore.h"
#include "components/resources/metadata.h"
#include "components/datasources/imagesource.h"
#include "components/datasources/geomsource.h"
#include "components/datasources/uddatasource.h"
#include "components/activities/activity.h"
#include "components/activities/viewer.h"
#include "components/plugin/componentplugin.h"
#include "renderscene.h"
#include "eplua.h"

#include "udPlatformUtil.h"

namespace ep
{
epResult udRenderScene_Init(Kernel*);
epResult udRenderScene_InitRender(Kernel*);

epResult udRenderScene_Deinit(Kernel*);
epResult udRenderScene_DeinitRender(Kernel*); // Not sure if both Deinit's are necessary

Kernel::Kernel()
  : componentRegistry(256)
  , instanceRegistry(256)
  , foreignInstanceRegistry(256)
  , messageHandlers(64)
{
}

epResult Kernel::Create(Kernel **ppInstance, InitParams commandLine, int renderThreadCount)
{
  epResult result;
  StreamRef spDebugFile, spConsole;
  Kernel *pKernel = CreateInstanceInternal(commandLine);

  EP_ERROR_NULL(pKernel, epR_Failure);

  pKernel->pRenderer = new Renderer(pKernel, renderThreadCount);

  // register all the builtin component types
  EP_ERROR_CHECK(pKernel->RegisterComponent<Component>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<DataSource>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Stream>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<File>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Console>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<MemStream>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Logger>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<PluginManager>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<PluginLoader>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<NativePluginLoader>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<ResourceManager>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<ShortcutManager>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Project>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Timer>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Lua>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<UIComponent>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<UIConsole>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Viewport>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Window>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<View>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Scene>());

  // nodes
  EP_ERROR_CHECK(pKernel->RegisterComponent<Node>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Camera>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<SimpleCamera>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<GeomNode>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<UDNode>());

  // resources
  EP_ERROR_CHECK(pKernel->RegisterComponent<Resource>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Buffer>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<ArrayBuffer>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<UDModel>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Shader>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Material>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Model>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Text>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<KVPStore>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Metadata>());

  // data sources
  EP_ERROR_CHECK(pKernel->RegisterComponent<ImageSource>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<GeomSource>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<UDDataSource>());

  // activities
  EP_ERROR_CHECK(pKernel->RegisterComponent<Activity>());
  EP_ERROR_CHECK(pKernel->RegisterComponent<Viewer>());

  // plugin interfaces
  EP_ERROR_CHECK(pKernel->RegisterComponent<ComponentPlugin>());

  // init the HAL
  EP_ERROR_CHECK(epHAL_Init());

  // create internal stuff
  pKernel->spLua = pKernel->CreateComponent<Lua>();

  // create logger and default streams
  pKernel->spLogger = pKernel->CreateComponent<Logger>();

  spDebugFile = pKernel->CreateComponent<File>({ { "path", "epKernel.log" }, { "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });
  if (spDebugFile)
  {
    pKernel->spLogger->AddStream(spDebugFile);
    spDebugFile->WriteLn("\n*** Logging started ***");
  }

  spConsole = pKernel->CreateComponent<Console>({ { "output", ConsoleOutputs::StdDbg } });
  if (spConsole)
     pKernel->spLogger->AddStream(spConsole);

  pKernel->spLogger->SetLevel(LogCategories::Trace, 0);

  // resource manager
  pKernel->spResourceManager = pKernel->CreateComponent<ResourceManager>();

  // shortcut manager
  pKernel->spShortcutManager = pKernel->CreateComponent<ShortcutManager>();

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

  epResult result = epR_Success;
  epResult renderSceneRenderResult = udRenderScene_DeinitRender(this);
  epResult renderSceneResult = udRenderScene_Deinit(this);

  // unregister components, free stuff
  //...

  // TODO: Destroy the streamer timer
  // pKernel->spStreamerTimer;

  udOctree_Shutdown();

  // TODO: fix!
  //delete pRenderer;

  epHAL_Deinit();

  delete this;

  if (renderSceneRenderResult != epR_Success)
    result = renderSceneRenderResult;
  else if (renderSceneResult != epR_Success)
    result = renderSceneResult;

  return result;
}

void Kernel::Update()
{
  static uint64_t last = udPerfCounterStart();
  uint64_t now = udPerfCounterStart();
  double sec = (double)udPerfCounterMilliseconds(last, now) / 1000.0;
  last = now;

  UpdatePulse.Signal(sec);
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

Array<const ComponentDesc *> Kernel::GetDerivedComponentDescs(const ComponentDesc *pBase, bool bIncludeBase)
{
  Array<const ComponentDesc *> derivedDescs;

  for (ComponentType &ct : componentRegistry)
  {
    const ComponentDesc *pDesc = ct.pDesc;
    if(!bIncludeBase)
      pDesc = pDesc->pSuperDesc;

    while (pDesc)
    {
      if (pDesc == pBase)
      {
        derivedDescs.concat(ct.pDesc);
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
    Component **ppComponent = instanceRegistry.Get(target.hash());
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
    MessageCallback *pHandler = messageHandlers.Get(target.hash());
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
  MessageCallback handler;
  handler.name = name;
  handler.callback = messageHandler;
  messageHandlers.Add(name.hash(), handler);
}

epResult Kernel::RegisterComponentType(ComponentDesc *pDesc)
{
  if (pDesc->id.exists('@') || pDesc->id.exists('$') || pDesc->id.exists('#'))
  {
    EPASSERT(false, "Invalid component id");
    return epR_Failure;
  }

  // build search trees
  pDesc->BuildSearchTrees();

  // add to registry
  ComponentType t = { pDesc, 0 };
  componentRegistry.Add(pDesc->id.hash(), t);
  return epR_Success;
}

const ComponentDesc* Kernel::GetComponentDesc(String id)
{
  ComponentType *pCT = componentRegistry.Get(id.hash());
  if (!pCT)
    return nullptr;
  return pCT->pDesc;
}

epResult Kernel::CreateComponent(String typeId, InitParams initParams, ComponentRef *pNewInstance)
{
  ComponentType *pType = componentRegistry.Get(typeId.hash());
  if (!pType)
    return epR_Failure;

  try
  {
    const ComponentDesc *pDesc = pType->pDesc;

    // TODO: should we have a better uid generator than this?
    MutableString64 newUid; newUid.concat(pDesc->id, pType->createCount++);

    ComponentRef spComponent(pDesc->pCreateInstance(pDesc, this, newUid, initParams));
    if (!spComponent)
      return epR_MemoryAllocationFailure;

    spComponent->Init(initParams);

    instanceRegistry.Add(spComponent->uid.hash(), spComponent.ptr());

    if (spLua)
      spLua->SetGlobal(spComponent->uid, spComponent.ptr());

    // TODO: inform partner kernels that I created a component
    //...

    *pNewInstance = spComponent;
    return epR_Success;
  }
  catch (epResult r)
  {
    LogDebug(3, "Create component failed!");
    return r;
  }
  catch (...)
  {
    LogDebug(3, "Create component failed!");
    return epR_Failure;
  }
}

epResult Kernel::DestroyComponent(Component *pInstance)
{
  spLua->SetGlobal(pInstance->uid, nullptr);

  // TODO: remove from component registry
  instanceRegistry.Destroy(pInstance->uid.hash());

  // TODO: inform partners that I destroyed a component
  //...

  return epR_Success;
}

ComponentRef Kernel::FindComponent(String _uid) const
{
  if (_uid.empty() || _uid[0] == '$' || _uid[0] == '#')
    return nullptr;
  if (_uid[0] == '@')
    _uid.popFront();
  Component **ppComponent = instanceRegistry.Get(_uid.toStringz());
  return ppComponent ? ComponentRef(*ppComponent) : nullptr;
}

epResult Kernel::InitComponents()
{
  epResult r = epR_Success;
  for (auto i : componentRegistry)
  {
    if (i.pDesc->pInit)
    {
      r = i.pDesc->pInit(this);
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

epResult Kernel::RegisterExtensions(const ComponentDesc *pDesc, const Slice<const String> exts)
{
  for (const String &e : exts)
  {
    extensionsRegistry.Insert(e, pDesc);
  }

  return epR_Success;
}

DataSourceRef Kernel::CreateDataSourceFromExtension(String ext, InitParams initParams)
{
  const ComponentDesc **pDesc = extensionsRegistry.Get(ext);
  if (!pDesc)
    return nullptr;

  ComponentRef spNewDataSource = nullptr;
  epResult r = CreateComponent((*pDesc)->id, initParams, &spNewDataSource);
  if (r != epR_Success)
    return nullptr;

  return shared_pointer_cast<DataSource>(spNewDataSource);
}

} // namespace ep

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

