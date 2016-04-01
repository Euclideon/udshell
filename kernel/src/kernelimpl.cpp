#include "ep/cpp/platform.h"
#include "ep/cpp/plugin.h"

#include "kernelimpl.h"

#include "hal/hal.h"
#include "components/file.h"
#include "components/stdiostream.h"
#include "components/memstream.h"
#include "components/lua.h"
#include "components/logger.h"
#include "components/timer.h"
#include "components/nodes/geomnode.h"
#include "components/pluginmanager.h"
#include "components/pluginloader.h"
#include "components/nativepluginloader.h"
#include "components/project.h"
#include "ep/cpp/component/resource/kvpstore.h"
#include "components/resources/text.h"
#include "components/datasources/imagesource.h"
#include "components/datasources/geomsource.h"
#include "components/datasources/uddatasource.h"
#include "components/console.h"

// Components that do the Impl dance
#include "components/componentimpl.h"
#include "components/viewimpl.h"
#include "components/commandmanagerimpl.h"
#include "components/resourcemanagerimpl.h"
#include "components/activityimpl.h"
#include "components/resources/metadataimpl.h"
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

#include "components/dynamiccomponent.h"
#include "components/varcomponent.h"

#include "components/glue/componentglue.h"

#include "renderscene.h"
#include "eplua.h"
#include "stdcapture.h"
#include "hal/haldirectory.h"

#include "udPlatformUtil.h"
#include "helpers.h"

namespace ep {

namespace internal {
  void *_Alloc(size_t size, epAllocationFlags flags, const char * pFile, int line);
  void *_AllocAligned(size_t size, size_t alignment, epAllocationFlags flags, const char * pFile, int line);
  void _Free(void *pMemory);
}

static Instance s_instance =
{
  EP_APIVERSION,  // apiVersion;
  nullptr,        // pKernelInstance;

  [](size_t size, epAllocationFlags flags, const char *pFile, int line) -> void* { return internal::_Alloc(size, flags, pFile, line); }, //  Alloc

  [](size_t size, size_t alignment, epAllocationFlags flags, const char *pFile, int line) -> void* { return internal::_AllocAligned(size, alignment, flags, pFile, line); }, // AllocAligned

  [](void *pMem) -> void { internal::_Free(pMem); }, // Free

  [](String condition, String message, String file, int line) -> void { IF_EPASSERT(epAssertFailed(condition, message, file, line);) }, // AssertFailed

  // NOTE: this was called when an RC reached zero...
  [](Component *pInstance) -> void { pInstance->DecRef(); }, // DestroyComponent, dec it with the internal function which actually performs the cleanup

  []() -> void* { return (void*)&KernelImpl::s_varAVLAllocator; }, // TreeAllocator
};

struct GlobalInstanceInitializer
{
  GlobalInstanceInitializer()
  {
    ep::s_pInstance = &s_instance;
  }
};

GlobalInstanceInitializer globalInstanceInitializer;

ComponentDescInl *Kernel::MakeKernelDescriptor(ComponentDescInl *pType)
{
  ComponentDescInl *pDesc = epNew ComponentDescInl;
  EPTHROW_IF_NULL(pDesc, epR_AllocFailure, "Memory allocation failed");

  pDesc->info = Kernel::MakeDescriptor();
  pDesc->info.flags = ComponentInfoFlags::Unregistered;
  pDesc->baseClass = Component::ComponentID();

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pSuperDesc = nullptr;

  // build search trees
  for (auto &p : CreateHelper<Kernel>::GetProperties())
    pDesc->propertyTree.Insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : CreateHelper<Kernel>::GetMethods())
    pDesc->methodTree.Insert(m.id, { m, m.pMethod });
  for (auto &e : CreateHelper<Kernel>::GetEvents())
    pDesc->eventTree.Insert(e.id, { e, e.pSubscribe });
  for (auto &f : CreateHelper<Kernel>::GetStaticFuncs())
    pDesc->staticFuncTree.Insert(f.id, { f, (void*)f.pCall });

  if (pType)
  {
    pType->pSuperDesc = pDesc;
    // populate the derived kernel from the base
    pType->PopulateFromDesc(pDesc);
    pDesc = pType;
  }
  return pDesc;
}
Kernel::Kernel(ComponentDescInl *_pType, Variant::VarMap commandLine)
  : Component(Kernel::MakeKernelDescriptor(_pType), nullptr, "kernel0", commandLine)
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
}

Kernel* Kernel::CreateInstance(Variant::VarMap commandLine, int renderThreadCount)
{
  if (!commandLine.Get("renderThreadCount"))
    commandLine.Insert("renderThreadCount", renderThreadCount);

  return CreateInstanceInternal(commandLine);
}


AVLTreeAllocator<VariantAVLNode> KernelImpl::s_varAVLAllocator;

KernelImpl::KernelImpl(Kernel *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
  , componentRegistry(256)
  , glueRegistry(64)
  , instanceRegistry(8192)
  , namedInstanceRegistry(4096)
  , foreignInstanceRegistry(4096)
  , messageHandlers(64)
{
  s_pInstance->pKernelInstance = pInstance;
}

void KernelImpl::StartInit(Variant::VarMap initParams)
{
  // init the kernel
  epscope(fail) { epDebugFormat("Error creating Kernel\n"); };

  renderThreadCount = initParams["renderThreadCount"].as<int>();

  // register the base Component type
  pInstance->RegisterComponentType<Component, ComponentImpl, ComponentGlue>();

  // HACK: update the descriptor with the base class (bootup chicken/egg)
  const ComponentDescInl *pComponentBase = componentRegistry.Get(Component::ComponentID())->pDesc;
  ComponentDescInl *pDesc = (ComponentDescInl*)pInstance->pType;
  while (pDesc->pSuperDesc)
  {
    // make sure each component in the kernel hierarchy get all the component meta
    pDesc->PopulateFromDesc(pComponentBase);
    pDesc = (ComponentDescInl*)pDesc->pSuperDesc;
  }
  pDesc->pSuperDesc = pComponentBase;

  // HACK: fix up the base class since we have a kernel instance (bootup chicken/egg)
  (Kernel*&)pInstance->pKernel = pInstance;
  pInstance->Component::pImpl = pInstance->Component::CreateImpl(initParams);

  // register all the builtin component types
  pInstance->RegisterComponentType<DataSource, DataSourceImpl>();
  pInstance->RegisterComponentType<Broadcaster, BroadcasterImpl>();
  pInstance->RegisterComponentType<Stream, StreamImpl>();
  pInstance->RegisterComponentType<File>();
  pInstance->RegisterComponentType<StdIOStream>();
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
  pInstance->RegisterComponentType<View, ViewImpl>();
  pInstance->RegisterComponentType<Activity, ActivityImpl>();
  pInstance->RegisterComponentType<Console>();

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
  pInstance->RegisterComponentType<Metadata, MetadataImpl>();
  pInstance->RegisterComponentType<Scene, SceneImpl>();

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

  // dynamic components
  pInstance->RegisterComponentType<DynamicComponent>();
  pInstance->RegisterComponentType<VarComponent>();

  // init the HAL
  EPTHROW_RESULT(epHAL_Init(), "epHAL_Init() failed");

  // create logger and default streams
  spLogger = pInstance->CreateComponent<Logger>();
  spLogger->DisableCategory(LogCategories::Trace);

  StreamRef spDebugFile = pInstance->CreateComponent<File>({ { "name", "logfile" }, { "path", "epKernel.log" }, { "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });
  spLogger->AddStream(spDebugFile);
  spDebugFile->WriteLn("\n*** Logging started ***");

#if EP_DEBUG
  StreamRef spStdIOStream = pInstance->CreateComponent<StdIOStream>({ { "output", StdIOStreamOutputs::StdDbg }, {"name", "debugout"} });
  spLogger->AddStream(spStdIOStream);
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

Array<SharedString> KernelImpl::ScanPluginFolder(String folderPath)
{
  EPFindData findData;
  EPFind find;
  Array<SharedString> pluginFilenames;

  if (!HalDirectory_FindFirst(&find, folderPath.ptr, &findData))
    return nullptr;
  do
  {
    if (findData.attributes & EPFA_Directory)
    {
      MutableString<260> childFolderPath(Format, "{0}/{1}", folderPath, String(findData.pFilename));

      Array<SharedString> childNames = ScanPluginFolder(childFolderPath);
      for (SharedString &cName : childNames)
        pluginFilenames.pushBack(std::move(cName));
    }
    else
      pluginFilenames.pushBack(MutableString<260>(Format, "{0}/{1}", folderPath, String(findData.pFilename)));
  } while (HalDirectory_FindNext(&find, &findData));

  HalDirectory_FindClose(&find);

  return pluginFilenames;
}

void KernelImpl::LoadPlugins()
{
  Array<SharedString> pluginFilenames = ScanPluginFolder("bin/plugins");

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

Array<const ComponentDesc *> KernelImpl::GetDerivedComponentDescs(String id, bool bIncludeBase)
{
  ComponentType *compType = componentRegistry.Get(id);
  if (compType)
    return GetDerivedComponentDescs(compType->pDesc, bIncludeBase);
  else
    return nullptr;
}

Array<const ComponentDesc *> KernelImpl::GetDerivedComponentDescs(const ComponentDesc *pBase, bool bIncludeBase)
{
  Array<const ComponentDesc *> derivedDescs;

  for (auto ct : componentRegistry)
  {
    const ComponentDesc *pDesc = ct.value.pDesc;
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

const ComponentDesc* KernelImpl::RegisterComponentType(ComponentDescInl *pDesc)
{
  if (pDesc->info.id.exists('@') || pDesc->info.id.exists('$') || pDesc->info.id.exists('#'))
    EPTHROW_ERROR(epR_InvalidArgument, "Invalid component id");

  // disallow duplicates
  if (componentRegistry.Get(pDesc->info.id))
    EPTHROW_ERROR(epR_InvalidArgument, "Component of type id '{0}' has already been registered", pDesc->info.id);

  // add to registry
  componentRegistry.Insert(pDesc->info.id, ComponentType{ pDesc, 0 });

  if (bKernelCreated && pDesc->pInit)
    pDesc->pInit(pInstance);

  return pDesc;
}

const ComponentDesc* KernelImpl::RegisterComponentType(Variant::VarMap typeDesc)
{
  DynamicComponentDesc *pDesc = epNew DynamicComponentDesc;

  pDesc->info.id = typeDesc["id"].asSharedString();
  pDesc->info.displayName = typeDesc["name"].asSharedString();
  pDesc->info.description = typeDesc["description"].asSharedString();
  pDesc->info.epVersion = EP_APIVERSION;
  Variant *pVar = typeDesc.Get("version");
  pDesc->info.pluginVersion = pVar ? pVar->as<int>() : EPKERNEL_PLUGINVERSION;
  pVar = typeDesc.Get("flags");
  pDesc->info.flags = pVar ? pVar->as<ComponentInfoFlags>() : ComponentInfoFlags();

  pDesc->baseClass = typeDesc["super"].asSharedString();

  pDesc->pInit = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pCreateInstance = [](const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams) -> ComponentRef {
    MutableString128 t(Format, "New (From VarMap): {0} - {1}", _pType->info.id, _uid);
    _pKernel->LogDebug(4, t);
    const DynamicComponentDesc *pDesc = (const DynamicComponentDesc*)_pType;
    DynamicComponentRef spInstance = pDesc->newInstance(KernelRef(_pKernel), initParams);
    ComponentRef spC = _pKernel->CreateGlue(pDesc->baseClass, _pType, _uid, spInstance, initParams);
    spInstance->AttachToGlue(spC.ptr());
    spC->pUserData = spInstance->GetUserData();
    return spC;
  };

  pDesc->newInstance = typeDesc["new"].as<DynamicComponentDesc::NewInstanceFunc>();
  pDesc->userData = typeDesc["userdata"].asSharedPtr();

  // TODO: populate trees from stuff in dynamic descriptor
//  pDesc->desc.Get
/*
  // build search trees
  for (auto &p : CreateHelper<_ComponentType>::GetProperties())
    pDesc->propertyTree.Insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : CreateHelper<_ComponentType>::GetMethods())
    pDesc->methodTree.Insert(m.id, { m, m.pMethod });
  for (auto &e : CreateHelper<_ComponentType>::GetEvents())
    pDesc->eventTree.Insert(e.id, { e, e.pSubscribe });
  for (auto &f : CreateHelper<_ComponentType>::GetStaticFuncs())
    pDesc->staticFuncTree.Insert(f.id, { f, (void*)f.pCall });
*/

  // setup the super class and populate from its meta
  pDesc->pSuperDesc = GetComponentDesc(pDesc->baseClass);
  EPTHROW_IF(!pDesc->pSuperDesc, epR_InvalidType, "Base Component '{0}' not registered", pDesc->baseClass);
  pDesc->PopulateFromDesc((ComponentDescInl*)pDesc->pSuperDesc);

  return RegisterComponentType(pDesc);
}

void KernelImpl::RegisterGlueType(String name, CreateGlueFunc *pCreateFunc)
{
  glueRegistry.Insert(name, pCreateFunc);
}

void* KernelImpl::CreateImpl(String componentType, Component *_pInstance, Variant::VarMap initParams)
{
  ComponentDescInl *pDesc = (ComponentDescInl*)GetComponentDesc(componentType);
  if (pDesc->pCreateImpl)
    return pDesc->pCreateImpl(_pInstance, initParams);
  return nullptr;
}

const ComponentDesc* KernelImpl::GetComponentDesc(String id)
{
  ComponentType *pCT = componentRegistry.Get(id);
  if (!pCT)
    return nullptr;
  return pCT->pDesc;
}

ComponentRef KernelImpl::CreateComponent(String typeId, Variant::VarMap initParams)
{
  ComponentType *_pType = componentRegistry.Get(typeId);
  EPASSERT_THROW(_pType, epR_InvalidArgument, "typeId failed to lookup ComponentType");
  EPTHROW_IF(_pType->pDesc->info.flags & ComponentInfoFlags::Abstract, epR_InvalidType, "Cannot create component of abstract type '{0}'", typeId);

  try
  {
    const ComponentDescInl *pDesc = _pType->pDesc;

    // TODO: should we have a better uid generator than this?
    MutableString64 newUid(Concat, pDesc->info.id, _pType->createCount++);

    // attempt to create an instance
    ComponentRef spComponent(pDesc->pCreateInstance(pDesc, pInstance, newUid, initParams));

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

ComponentRef KernelImpl::CreateGlue(String typeId, const ComponentDesc *_pType, SharedString _uid, ComponentRef spInstance, Variant::VarMap initParams)
{
  CreateGlueFunc **ppCreate = glueRegistry.Get(typeId);
  EPTHROW_IF_NULL(ppCreate, epR_InvalidType, "No glue type {0}", typeId);
  return (*ppCreate)(pInstance, _pType, _uid, spInstance, initParams);
}

void KernelImpl::DestroyComponent(Component *_pInstance)
{
  if (_pInstance->name)
    namedInstanceRegistry.Remove(_pInstance->name);
  instanceRegistry.Remove(_pInstance->uid);

  // TODO: inform partners that I destroyed a component
  //...
}

ComponentRef KernelImpl::FindComponent(String _name) const
{
  if (_name.empty() || _name[0] == '$' || _name[0] == '#')
    return nullptr;
  if (_name[0] == '@')
    _name.popFront();
  Component * const * ppComponent = namedInstanceRegistry.Get(_name);
  if (!ppComponent)
    ppComponent = instanceRegistry.Get(_name);
  return ppComponent ? ComponentRef(*ppComponent) : nullptr;
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

const AVLTree<String, const ComponentDesc *> &KernelImpl::GetExtensionsRegistry() const
{
  return extensionsRegistry;
}

void KernelImpl::RegisterExtensions(const ComponentDesc *pDesc, const Slice<const String> exts)
{
  for (const String &e : exts)
    extensionsRegistry.Insert(e, pDesc);
}

DataSourceRef KernelImpl::CreateDataSourceFromExtension(String ext, Variant::VarMap initParams)
{
  const ComponentDesc **ppDesc = extensionsRegistry.Get(ext);
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
