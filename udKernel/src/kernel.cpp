#include "udPlatform.h"
#include "kernel.h"
#include "udRender.h"
#include "udBlockStreamer.h"

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
#include "components/viewport.h"
#include "components/window.h"
#include "components/datasource.h"
#include "components/nodes/node.h"
#include "components/nodes/camera.h"
#include "components/nodes/geomnode.h"
#include "components/nodes/udnode.h"
#include "components/resourcemanager.h"
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
#include "renderscene.h"
#include "udlua.h"

namespace ud
{
void udRenderScene_Init(Kernel*);
void udRenderScene_InitRender(Kernel*);


udResult Kernel::Create(Kernel **ppInstance, udInitParams commandLine, int renderThreadCount)
{
  udResult result;
  StreamRef spDebugFile, spConsole;
  Kernel *pKernel = CreateInstanceInternal(commandLine);

  UD_ERROR_NULL(pKernel, udR_Failure_);

  pKernel->componentRegistry.Init(256);
  pKernel->instanceRegistry.Init(256);
  pKernel->foreignInstanceRegistry.Init(256);
  pKernel->messageHandlers.Init(64);

  pKernel->pRenderer = udNew(Renderer, pKernel, renderThreadCount);

  // register all the builtin component types
  UD_ERROR_CHECK(pKernel->RegisterComponent<Component>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<DataSource>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Stream>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<File>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Console>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<MemStream>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Logger>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<ResourceManager>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Timer>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Lua>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<UIComponent>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Viewport>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Window>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<View>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Scene>());

  // nodes
  UD_ERROR_CHECK(pKernel->RegisterComponent<Node>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Camera>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<SimpleCamera>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<GeomNode>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<UDNode>());

  // resources
  UD_ERROR_CHECK(pKernel->RegisterComponent<Resource>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Buffer>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<ArrayBuffer>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<UDModel>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Shader>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Material>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Model>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Text>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<KVPStore>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<Metadata>());

  // data sources
  UD_ERROR_CHECK(pKernel->RegisterComponent<ImageSource>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<GeomSource>());
  UD_ERROR_CHECK(pKernel->RegisterComponent<UDDataSource>());

  // init the HAL
  UD_ERROR_CHECK(udHAL_Init());

  // create internal stuff
  pKernel->spLua = pKernel->CreateComponent<Lua>();

  // create logger and default streams
  pKernel->spLogger = pKernel->CreateComponent<Logger>();

  spDebugFile = pKernel->CreateComponent<File>({ { "path", "udKernel.log" }, { "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create } });
  if (spDebugFile)
     pKernel->spLogger->AddStream(spDebugFile, LogCategories::Error | LogCategories::Warning | LogCategories::Debug | LogCategories::Info | LogCategories::Script | LogCategories::Trace, 5, LogDefaults::Format);

  spConsole = pKernel->CreateComponent<Console>({ { "output", ConsoleOutputs::StdDbg } });
  if (spConsole)
     pKernel->spLogger->AddStream(spConsole, LogCategories::Error | LogCategories::Warning | LogCategories::Debug | LogCategories::Info | LogCategories::Script, 5, LogDefaults::Format);

  // Resource Manager
  pKernel->spResourceManager = pKernel->CreateComponent<ResourceManager>();

  // platform init
  UD_ERROR_CHECK(pKernel->InitInstanceInternal());

epilogue:
  if (result != udR_Success)
  {
    // TODO: clean up code
  }
  *ppInstance = pKernel;
  return result;
}

void Kernel::DoInit(Kernel *pKernel)
{
  // init the components
  if (pKernel->InitComponents() != udR_Success)
  {
    UDASSERT(false, "Oh no! Can't boot!");
    return;
  }

  udRenderScene_Init(pKernel);
  udRenderScene_InitRender(pKernel);

  pKernel->spStreamerTimer = pKernel->CreateComponent<Timer>({ { "duration", 33 }, { "timertype", "Interval" } });
  pKernel->spStreamerTimer->Event.Subscribe(FastDelegate<void()>(pKernel, &Kernel::StreamerUpdate));

  pKernel->spUpdateTimer = pKernel->CreateComponent<Timer>({ { "duration", 16 }, { "timertype", "Interval" } });
  pKernel->spUpdateTimer->Event.Subscribe(FastDelegate<void()>(pKernel, &Kernel::Update));

  // call application init
  SendMessage("$init", "#", "init", nullptr);
}


udResult Kernel::Destroy()
{
  udResult result;

  // unregister components, free stuff
  //...

  // TODO: this crashes - fix!
  //udDelete(pLua);

  // TODO: Destroy the streamer timer
  // pKernel->spStreamerTimer;

  messageHandlers.Deinit();
  componentRegistry.Deinit();
  instanceRegistry.Deinit();
  foreignInstanceRegistry.Deinit();

  udOctree_Shutdown();

  delete pRenderer;

  udHAL_Deinit();

  UD_ERROR_CHECK(DestroyInstanceInternal());

epilogue:
  return result;
}

void Kernel::Update()
{
  static uint64_t last = udPerfCounterStart();
  uint64_t now = udPerfCounterStart();
  double sec = (double)udPerfCounterMilliseconds(last, now) / 1000.0;
  last = now;

  // TODO: this shouldn't require a focus view! get the scene from the project...

  SceneRef spScene = spFocusView->GetScene();
  if (spScene)
    spScene->Update(sec);
  CameraRef spCamera = spFocusView->GetCamera();
  if (spCamera)
  {
    if (spCamera->Update(sec))
      spFocusView->ForceDirty();
  }
}

void Kernel::StreamerUpdate()
{
  udStreamerStatus streamerStatus = { 0 };
  udOctree_Update(&streamerStatus);
}

udFixedSlice<const ComponentDesc *> Kernel::GetDerivedComponentDescs(const ComponentDesc *pBase, bool bIncludeBase)
{
  udFixedSlice<const ComponentDesc *> derivedDescs;

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

udResult Kernel::SendMessage(udString target, udString sender, udString message, const udVariant &data)
{
  if (target.empty())
    return udR_Failure_; // TODO: no target!!

  char targetType = target.popFront();
  if (targetType == '@')
  {
    // component message
    ComponentRef *pComponent = instanceRegistry.Get(target.hash());
    if (pComponent)
    {
      return (*pComponent)->ReceiveMessage(message, sender, data);
    }
    else
    {
      // TODO: check if it's in the foreign component registry and send it there

      return udR_Failure_; // TODO: no component!
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

      return udR_Failure_; // TODO: invalid kernel!
    }
  }
  else if (targetType == '$')
  {
    // registered message
    MessageCallback *pHandler = messageHandlers.Get(target.hash());
    if (pHandler)
    {
      pHandler->callback(sender, message, data);
      return udR_Success;
    }
    else
      return udR_Failure_; // TODO: no message handler
  }

  return udR_Failure_; // TODO: error, invalid target!
}

udResult Kernel::ReceiveMessage(udString sender, udString message, const udVariant &data)
{

  return udR_Success;
}

void Kernel::RegisterMessageHandler(udSharedString name, MessageHandler messageHandler)
{
  MessageCallback handler;
  handler.name = name;
  handler.callback = messageHandler;
  messageHandlers.Add(name.hash(), handler);
}

udResult Kernel::RegisterComponentType(ComponentDesc *pDesc)
{
  if (pDesc->id.exists('@') || pDesc->id.exists('$') || pDesc->id.exists('#'))
  {
    UDASSERT(false, "Invalid component id");
    return udR_Failure_;
  }

  // build search trees
  pDesc->BuildSearchTrees();

  // add to registry
  ComponentType t = { pDesc, 0 };
  componentRegistry.Add(pDesc->id.hash(), t);
  return udR_Success;
}

udResult Kernel::CreateComponent(udString typeId, udInitParams initParams, ComponentRef *pNewInstance)
{
  ComponentType *pType = componentRegistry.Get(typeId.hash());
  if (!pType)
    return udR_Failure_;

  try
  {
    const ComponentDesc *pDesc = pType->pDesc;

    // TODO: should we have a better uid generator than this?
    udMutableString64 uid; uid.concat(pDesc->id, pType->createCount++);

    ComponentRef spComponent(pDesc->pCreateInstance(pDesc, this, uid, initParams));
    if (!spComponent)
      return udR_MemoryAllocationFailure;

    spComponent->Init(initParams);

    instanceRegistry.Add(spComponent->uid.hash(), spComponent);

    if (spLua)
      spLua->SetGlobal(spComponent, udString(spComponent->uid));

    // TODO: inform partner kernels that I created a component
    //...

    *pNewInstance = spComponent;
    return udR_Success;
  }
  catch (udResult r)
  {
    return r;
  }
  catch (...)
  {
    return udR_Failure_;
  }
}

udResult Kernel::DestroyComponent(ComponentRef *pInstance)
{
  spLua->SetGlobal(nullptr, udString((*pInstance)->uid));

  // TODO: remove from component registry
  //instanceRegistry.Destroy((*pInstance)->uid.toStringz());

  // TODO: inform partners that I destroyed a component
  //...

  pInstance->reset();
  pInstance = nullptr;
  return udR_Success;
}

ComponentRef Kernel::FindComponent(udString uid)
{
  if (uid.empty() || uid[0] == '$' || uid[0] == '#')
    return nullptr;
  if (uid[0] == '@')
    uid.popFront();
  ComponentRef *pComponent = instanceRegistry.Get(uid.toStringz());
  return pComponent ? *pComponent : nullptr;
}

udResult Kernel::InitComponents()
{
  udResult r = udR_Success;
  for (auto i : componentRegistry)
  {
    if (i.pDesc->pInit)
    {
      r = i.pDesc->pInit(this);
      if (r != udR_Success)
        break;
    }
  }
  return r;
}

udResult Kernel::InitRender()
{
  udHAL_InitRender();

  return udR_Success;
}

udResult Kernel::DeinitRender()
{
  udHAL_DeinitRender();

  return udR_Success;
}

void Kernel::Exec(udString code)
{
  spLua->Execute(code);
}

// Helper functions for the kernel's logger
void Kernel::LogError(udString text, udString componentUID) { spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Error, componentUID); }
void Kernel::LogWarning(int level, udString text, udString componentUID) { spLogger->Log(level, text, LogCategories::Warning, componentUID); }
void Kernel::LogDebug(int level, udString text, udString componentUID) { spLogger->Log(level, text, LogCategories::Debug, componentUID); }
void Kernel::LogInfo(int level, udString text, udString componentUID) { spLogger->Log(level, text, LogCategories::Info, componentUID); }
void Kernel::LogScript(udString text, udString componentUID) { spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Script, componentUID); }
void Kernel::LogTrace(udString text, udString componentUID) { spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Trace, componentUID); }
// Calls LogDebug() with level 2
void Kernel::Log(udString text, const udString componentUID) { spLogger->Log(LogDefaults::LogLevel, text, LogCategories::Debug, componentUID); }

udResult Kernel::RegisterExtensions(const ComponentDesc *pDesc, const udSlice<const udString> exts)
{
  for (const udString &e : exts)
  {
    extensionsRegistry.Insert(e, pDesc);
  }

  return udR_Success;
}

DataSourceRef Kernel::CreateDataSourceFromExtension(udString ext, udInitParams initParams)
{
  const ComponentDesc **pDesc = extensionsRegistry.Get(ext);
  if (!pDesc)
    return nullptr;

  ComponentRef spNewDataSource = nullptr;
  udResult r = CreateComponent((*pDesc)->id, initParams, &spNewDataSource);
  if (r != udR_Success)
    return nullptr;

  return shared_pointer_cast<DataSource>(spNewDataSource);
}

} // namespace ud

// synchronised pointer destroy function (it's here because there's no udsharedptr.cpp file)
template<class T>
void udSynchronisedPtr<T>::destroy()
{
  struct S
  {
    void Destroy(ud::Kernel *pKernel)
    {
      ((udSharedPtr<T>&)this)->release();
    }
  };

  pKernel->DispatchToMainThread(MakeDelegate((S*)this, &S::Destroy));
  pInstance = nullptr;
}

