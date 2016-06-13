#include "ep/cpp/platform.h"
#include "ep/cpp/plugin.h"

#include "kernelimpl.h"

#include "components/stdiostream.h"
#include "components/lua.h"
#include "components/logger.h"
#include "components/timerimpl.h"
#include "components/pluginmanager.h"
#include "components/pluginloader.h"
#include "components/nativepluginloader.h"
#include "ep/cpp/component/resource/kvpstore.h"
#include "components/datasources/imagesource.h"
#include "components/datasources/geomsource.h"
#include "components/datasources/udsource.h"
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
#include "components/resources/textimpl.h"
#include "components/nodes/nodeimpl.h"
#include "components/nodes/scenenodeimpl.h"
#include "components/nodes/udnodeimpl.h"
#include "components/nodes/cameraimpl.h"
#include "components/nodes/simplecameraimpl.h"
#include "components/nodes/geomnodeimpl.h"
#include "components/sceneimpl.h"
#include "components/datasources/datasourceimpl.h"
#include "components/broadcasterimpl.h"
#include "components/streamimpl.h"
#include "components/regeximpl.h"
#include "components/primitivegeneratorimpl.h"
#include "components/projectimpl.h"
#include "components/settingsimpl.h"
#include "components/fileimpl.h"
#include "components/memstreamimpl.h"

#include "components/dynamiccomponent.h"
#include "components/varcomponent.h"

#include "components/glue/componentglue.h"

#include "renderscene.h"
#include "eplua.h"
#include "stdcapture.h"

#include "hal/hal.h"
#include "hal/directory.h"

#include "udPlatformUtil.h"
#include "helpers.h"

#include "ep/cpp/filesystem.h"

namespace ep {

namespace internal {
  void *_Alloc(size_t size, epAllocationFlags flags, const char * pFile, int line);
  void _Free(void *pMemory);

  void TranslateFindData(const EPFindData &fd, ep::FindData *pFD)
  {
    pFD->filename = (const char*)fd.pFilename;
    pFD->path = (const char*)fd.pSystemPath;
    pFD->attributes = ((fd.attributes & EPFA_Directory) ? FileAttributes::Directory : 0) |
                      ((fd.attributes & EPFA_SymLink) ? FileAttributes::SymLink : 0) |
                      ((fd.attributes & EPFA_Hidden) ? FileAttributes::Hidden : 0) |
                      ((fd.attributes & EPFA_ReadOnly) ? FileAttributes::ReadOnly : 0);
    pFD->fileSize = fd.fileSize;
    pFD->accessTime.ticks = fd.accessTime.ticks;
    pFD->writeTime.ticks = fd.writeTime.ticks;
  }

  ErrorSystem s_errorSystem;

  struct Allocators
  {
    static void *GetVarAVLAllocator()
    {
      if (!KernelImpl::s_pVarAVLAllocator)
        KernelImpl::s_pVarAVLAllocator = epNew(KernelImpl::VarAVLTreeAllocator);
      return KernelImpl::s_pVarAVLAllocator;
    }

    static void *GetWeakRefRegistry()
    {
      if (!KernelImpl::s_pWeakRefRegistry)
        KernelImpl::s_pWeakRefRegistry = epNew(KernelImpl::WeakRefRegistryMap, 65536);
      return KernelImpl::s_pWeakRefRegistry;
    }

    static void *GetStaticImplRegistry()
    {
      if (!KernelImpl::s_pStaticImplRegistry)
        KernelImpl::s_pStaticImplRegistry = epNew(KernelImpl::StaticImplRegistryMap);
      return KernelImpl::s_pStaticImplRegistry;
    }

    static void Destroy()
    {
      epDelete(KernelImpl::s_pVarAVLAllocator);
      epDelete(KernelImpl::s_pStaticImplRegistry);
      epDelete(KernelImpl::s_pWeakRefRegistry);
    }
  };

} // internal

static Instance s_instance =
{
  EP_APIVERSION,  // apiVersion;

  nullptr,        // pKernelInstance;
  (void*)&internal::s_errorSystem, // ErrorSystem
  internal::Allocators::GetStaticImplRegistry(),
  internal::Allocators::GetVarAVLAllocator(),
  internal::Allocators::GetWeakRefRegistry(),

  [](size_t size, epAllocationFlags flags, const char *pFile, int line) -> void* { return internal::_Alloc(size, flags, pFile, line); }, //  Alloc

  [](void *pMem) -> void { internal::_Free(pMem); }, // Free

  [](String condition, String message, String file, int line) -> void { IF_EPASSERT(AssertFailed(condition, message, file, line);) }, // AssertFailed

  // NOTE: this was called when an RC reached zero...
  [](Component *pInstance) -> void { pInstance->DecRef(); }, // DestroyComponent, dec it with the internal function which actually performs the cleanup

  [](String pattern, void *pHandle, void *pData) -> void* {
    EPFind *pFind = (EPFind*)pHandle;
    FindData *pFD = (FindData*)pData;
    if (pattern && !pFind && pFD)
    {
      EPFind *find = epNew(EPFind);
      EPFindData fd;
      if (HalDirectory_FindFirst(find, pattern.toStringz(), &fd))
      {
        internal::TranslateFindData(fd, pFD);
        return find;
      }
      else
      {
        epDelete(find);
        return nullptr;
      }
    }
    else if (!pattern && pFind && pFD)
    {
      EPFindData fd;
      if (HalDirectory_FindNext(pFind, &fd))
      {
        internal::TranslateFindData(fd, pFD);
        return pHandle;
      }
      else
      {
        HalDirectory_FindClose(pFind);
        epDelete(pFind);
        return nullptr;
      }
    }
    else if (!pattern && pFind && !pFD)
    {
      HalDirectory_FindClose(pFind);
      epDelete(pFind);
      return nullptr;
    }
    else
      EPTHROW_ERROR(Result::InvalidArgument, "Bad call");
  } // Find
};

struct GlobalInstanceInitializer
{
  GlobalInstanceInitializer()
  {
    ep::s_pInstance = &s_instance;
  }

  ~GlobalInstanceInitializer()
  {
    internal::Allocators::Destroy();
  }
};

GlobalInstanceInitializer globalInstanceInitializer;

ComponentDescInl *Kernel::MakeKernelDescriptor(ComponentDescInl *pType)
{
  ComponentDescInl *pDesc = epNew(ComponentDescInl);
  EPTHROW_IF_NULL(pDesc, Result::AllocFailure, "Memory allocation failed");

  pDesc->info = Kernel::ComponentInfo();
  pDesc->info.flags = ComponentInfoFlags::Unregistered;
  pDesc->baseClass = Component::ComponentID();

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pSuperDesc = nullptr;

  // build search trees
  for (auto &p : CreateHelper<Kernel>::GetProperties())
    pDesc->propertyTree.insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : CreateHelper<Kernel>::GetMethods())
    pDesc->methodTree.insert(m.id, { m, m.pMethod });
  for (auto &e : CreateHelper<Kernel>::GetEvents())
    pDesc->eventTree.insert(e.id, { e, e.pSubscribe });
  for (auto &f : CreateHelper<Kernel>::GetStaticFuncs())
    pDesc->staticFuncTree.insert(f.id, { f, (void*)f.pCall });

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
  : Component(Kernel::MakeKernelDescriptor(_pType), nullptr, "ep.kernel0", commandLine)
{
  // alloc impl
  pImpl = UniquePtr<Impl>(epNew(KernelImpl, this, commandLine));
  GetImpl()->StartInit(commandLine);
}

Kernel::~Kernel()
{
  // HACK: undo chicken/egg hacks
  Component::pImpl = nullptr;
  (Kernel*&)pKernel = nullptr;

  // Unhook the registered components from our stack before they get deleted
  const ComponentDesc *pDesc = pType;
  while (pDesc)
  {
    if (pDesc->pSuperDesc && !(pDesc->pSuperDesc->info.flags & ComponentInfoFlags::Unregistered))
    {
      (const ComponentDesc*&)pDesc->pSuperDesc = nullptr;
      break;
    }
    pDesc = pDesc->pSuperDesc;
  }
}

Kernel* Kernel::CreateInstance(Variant::VarMap commandLine, int renderThreadCount)
{
  if (!commandLine.get("renderThreadCount"))
    commandLine.insert("renderThreadCount", renderThreadCount);

  return CreateInstanceInternal(commandLine);
}

KernelImpl::VarAVLTreeAllocator* KernelImpl::s_pVarAVLAllocator;
KernelImpl::WeakRefRegistryMap* KernelImpl::s_pWeakRefRegistry;
KernelImpl::StaticImplRegistryMap* KernelImpl::s_pStaticImplRegistry;

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
  epscope(fail) { DebugFormat("Error creating Kernel\n"); };

  renderThreadCount = initParams["renderThreadCount"].as<int>();

  // register the base Component type
  pInstance->RegisterComponentType<Component, ComponentImpl, ComponentGlue>();

  // HACK: update the descriptor with the base class (bootup chicken/egg)
  const ComponentDescInl *pComponentBase = componentRegistry.get(Component::ComponentID())->pDesc;
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
  pInstance->RegisterComponentType<File, FileImpl, void, FileImplStatic>();
  pInstance->RegisterComponentType<StdIOStream>();
  pInstance->RegisterComponentType<MemStream, MemStreamImpl>();
  pInstance->RegisterComponentType<Regex, RegexImpl>();
  pInstance->RegisterComponentType<Logger>();
  pInstance->RegisterComponentType<PluginManager>();
  pInstance->RegisterComponentType<PluginLoader>();
  pInstance->RegisterComponentType<NativePluginLoader>();
  pInstance->RegisterComponentType<ResourceManager, ResourceManagerImpl>();
  pInstance->RegisterComponentType<CommandManager, CommandManagerImpl>();
  pInstance->RegisterComponentType<Project, ProjectImpl>();
  pInstance->RegisterComponentType<Timer, TimerImpl>();
  pInstance->RegisterComponentType<Settings, SettingsImpl>();
  pInstance->RegisterComponentType<Lua>();
  pInstance->RegisterComponentType<View, ViewImpl>();
  pInstance->RegisterComponentType<Activity, ActivityImpl>();
  pInstance->RegisterComponentType<Console>();
  pInstance->RegisterComponentType<PrimitiveGenerator, PrimitiveGeneratorImpl>();

  // resources
  pInstance->RegisterComponentType<Resource, ResourceImpl>();
  pInstance->RegisterComponentType<Buffer, BufferImpl>();
  pInstance->RegisterComponentType<ArrayBuffer, ArrayBufferImpl>();
  pInstance->RegisterComponentType<UDModel, UDModelImpl>();
  pInstance->RegisterComponentType<Shader, ShaderImpl>();
  pInstance->RegisterComponentType<Material, MaterialImpl>();
  pInstance->RegisterComponentType<Model, ModelImpl>();
  pInstance->RegisterComponentType<Text, TextImpl, void, TextImplStatic>();
  pInstance->RegisterComponentType<Menu, MenuImpl>();
  pInstance->RegisterComponentType<KVPStore>();
  pInstance->RegisterComponentType<Metadata, MetadataImpl>();
  pInstance->RegisterComponentType<Scene, SceneImpl>();

  // nodes
  pInstance->RegisterComponentType<Node, NodeImpl>();
  pInstance->RegisterComponentType<SceneNode, SceneImpl>();
  pInstance->RegisterComponentType<Camera, CameraImpl>();
  pInstance->RegisterComponentType<SimpleCamera, SimpleCameraImpl>();
  pInstance->RegisterComponentType<GeomNode, GeomNodeImpl>();
  pInstance->RegisterComponentType<UDNode, UDNodeImpl>();

  // data sources
  pInstance->RegisterComponentType<ImageSource>();
  pInstance->RegisterComponentType<GeomSource>();
  pInstance->RegisterComponentType<UDSource>();

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

  // settings
  spSettings = pInstance->CreateComponent<Settings>({ { "name", "settings" }, { "src", "settings.epset" } });

  // plugin manager
  spPluginManager = pInstance->CreateComponent<PluginManager>({ { "name", "pluginmanager" } });
  spPluginManager->RegisterPluginLoader(pInstance->CreateComponent<NativePluginLoader>());

  // Init capture and broadcast of stdout/stderr
  spStdOutBC = pInstance->CreateComponent<Broadcaster>({ { "name", "stdoutbc" } });
  stdOutCapture = epNew(StdCapture, stdout);
  epscope(fail) { epDelete(stdOutCapture); };
  spStdErrBC = pInstance->CreateComponent<Broadcaster>({ { "name", "stderrbc" } });
  stdErrCapture = epNew(StdCapture, stderr);
  epscope(fail) { epDelete(stdErrCapture); };

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

  // call application register
  if (HasMessageHandler("register"))
    SendMessage("$register", "#", "register", nullptr);

  // load the plugins
  LoadPluginDir(Slice<const String>{ "bin/plugins", "plugins" });

  // make the kernel timers
  spStreamerTimer = pInstance->CreateComponent<Timer>({ { "duration", 33 },{ "timertype", "Interval" } });
  spStreamerTimer->Elapsed.Subscribe(FastDelegate<void()>(this, &KernelImpl::StreamerUpdate));

  spUpdateTimer = pInstance->CreateComponent<Timer>({ { "duration", 16 },{ "timertype", "Interval" } });
  spUpdateTimer->Elapsed.Subscribe(FastDelegate<void()>(this, &KernelImpl::Update));

  // call application init
  if (HasMessageHandler("init"))
    SendMessage("$init", "#", "init", nullptr);
}

KernelImpl::~KernelImpl()
{
  spLua = nullptr;

  spResourceManager = nullptr;
  spCommandManager = nullptr;

  epDelete (stdOutCapture);
  epDelete (stdErrCapture);
  stdOutCapture = nullptr;
  stdErrCapture = nullptr;

  spStdErrBC = nullptr;
  spStdOutBC = nullptr;

  spLogger = nullptr;
  spSettings = nullptr;

  if (instanceRegistry.begin() != instanceRegistry.end())
  {
    int count = 0;
    DebugFormat("!!!WARNING: Some Components have not been freed\n");

    for (const auto &c : instanceRegistry)
    {
      ++count;
      DebugFormat("Unfreed Component: {0} ({1}) refCount {2} \n", c.key, c.value->GetName(), c.value->RefCount());
    }
    DebugFormat("{0} Unfreed Component(s)\n", count);
  }

  epHAL_Deinit();

  for (const auto &c : componentRegistry)
    epDelete(c.value.pDesc);
}

void KernelImpl::Shutdown()
{
  // TODO: Consider whether or not to catch exceptions and then continuing the deinit path or just do nothing.

  if (spStreamerTimer)
    spStreamerTimer->Elapsed.Unsubscribe(FastDelegate<void()>(this, &KernelImpl::StreamerUpdate));
  if (spUpdateTimer)
    spUpdateTimer->Elapsed.Unsubscribe(FastDelegate<void()>(this, &KernelImpl::Update));

  // call application deinit
  if (HasMessageHandler("deinit"))
    SendMessage("$deinit", "#", "deinit", nullptr);

  SetFocusView(nullptr);

  spUpdateTimer = nullptr;
  spStreamerTimer = nullptr;

  spPluginManager = nullptr;

  spRenderer = nullptr;
}

Array<SharedString> KernelImpl::ScanPluginFolder(String folderPath, Slice<const String> extFilter)
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

      Array<SharedString> childNames = ScanPluginFolder(childFolderPath, extFilter);
      for (SharedString &cName : childNames)
        pluginFilenames.pushBack(std::move(cName));
    }
    else
    {
      bool valid = true;
      MutableString<260> filename(Format, "{0}/{1}", folderPath, String(findData.pFilename));
      for (auto &ext : extFilter)
      {
        valid = (filename.endsWithIC(ext));
        if (valid)
          break;
      }
      if (valid)
        pluginFilenames.pushBack(filename);
    }
  } while (HalDirectory_FindNext(&find, &findData));

  HalDirectory_FindClose(&find);

  return pluginFilenames;
}

void KernelImpl::LoadPluginDir(Slice<const String> folderPaths)
{
  for (auto path : folderPaths)
  {
    Array<SharedString> pluginFilenames = ScanPluginFolder(path);
    LoadPlugins(pluginFilenames);
  }
}

void KernelImpl::LoadPlugins(Slice<SharedString> files)
{
  size_t numRemaining = files.length;
  size_t lastTry;
  do
  {
    // since plugins may depend on other plugins, we'll keep trying to reload plugins while loads are succeeding
    lastTry = numRemaining;
    for (auto &filename : files)
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

  // output a warning if any plugins could not be loaded
  for (auto &filename : files)
  {
    if (filename)
      LogWarning(2, "Could not load plugin '{0}'", filename);
  }
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

Array<const ComponentDesc *> KernelImpl::GetDerivedComponentDescsFromString(String id, bool bIncludeBase)
{
  ComponentType *compType = componentRegistry.get(id);
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
  EPASSERT_THROW(!target.empty(), Result::InvalidArgument, "target was empty");

  char targetType = target.popFront();
  if (targetType == '@')
  {
    // component message
    Component **ppComponent = instanceRegistry.get(target);
    if (ppComponent)
    {
      ComponentRef spComponent(*ppComponent);
      try {
        spComponent->ReceiveMessage(message, sender, data);
      } catch (std::exception &e) {
        LogError("Message Handler {0} failed: {1}", target, e.what());
      } catch (...) {
        LogError("Message Handler {0} failed: C++ exception", target);
      }
    }
    else
    {
      // TODO: check if it's in the foreign component registry and send it there
      EPTHROW_ERROR(Result::Failure, "Target component not found");
    }
  }
  else if (targetType == '#')
  {
    // kernel message
    if (target.eq(uid))
    {
      // it's for me!
      try {
        ReceiveMessage(sender, message, data);
      } catch (std::exception &e) {
        LogError("Message Handler {0} failed: {1}", target, e.what());
      } catch (...) {
        LogError("Message Handler {0} failed: C++ exception", target);
      }
    }
    else
    {
      // TODO: foreign kernels?!
      EPTHROW_ERROR(Result::Failure, "Invalid Kernel");
    }
  }
  else if (targetType == '$')
  {
    // registered message
    MessageCallback *pHandler = messageHandlers.get(target);
    if (pHandler)
    {
      try {
        pHandler->callback(sender, message, data);
      } catch (std::exception &e) {
        LogError("Message Handler {0} failed: {1}", target, e.what());
      } catch (...) {
        LogError("Message Handler {0} failed: C++ exception", target);
      }
    }
    else
    {
      EPTHROW_ERROR(Result::Failure, "No Message Handler");
    }
  }
  else
  {
    EPTHROW_ERROR(Result::Failure, "Invalid target");
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
  messageHandlers.replace(_name, MessageCallback{ _name, messageHandler });
}

const ComponentDesc* KernelImpl::RegisterComponentType(ComponentDescInl *pDesc)
{
  if (pDesc->info.identifier.exists('@') || pDesc->info.identifier.exists('$') || pDesc->info.identifier.exists('#'))
    EPTHROW_ERROR(Result::InvalidArgument, "Invalid component id");

  // disallow duplicates
  if (componentRegistry.get(pDesc->info.identifier))
    EPTHROW_ERROR(Result::InvalidArgument, "Component of type id '{0}' has already been registered", pDesc->info.identifier);

  // add to registry
  componentRegistry.insert(pDesc->info.identifier, ComponentType{ pDesc, 0 });

  if (bKernelCreated && pDesc->pInit)
    pDesc->pInit(pInstance);

  return pDesc;
}

const ComponentDesc* KernelImpl::RegisterComponentTypeFromMap(Variant::VarMap typeDesc)
{
  DynamicComponentDesc *pDesc = epNew(DynamicComponentDesc);

  pDesc->info.identifier = typeDesc["identifier"].asSharedString();

  size_t offset = pDesc->info.identifier.findLast('.');
  EPTHROW_IF(offset == (size_t)-1, Result::InvalidArgument, "Component identifier {0} has no namespace. Use form: namespace.componentname", pDesc->info.identifier);

  pDesc->info.nameSpace = pDesc->info.identifier.slice(0, offset);
  pDesc->info.name = pDesc->info.identifier.slice(offset+1, pDesc->info.identifier.length);

  pDesc->info.displayName = typeDesc["name"].asSharedString();
  pDesc->info.description = typeDesc["description"].asSharedString();
  pDesc->info.epVersion = EP_APIVERSION;
  Variant *pVar = typeDesc.get("version");
  pDesc->info.pluginVersion = pVar ? pVar->as<int>() : EPKERNEL_PLUGINVERSION;
  pVar = typeDesc.get("flags");
  pDesc->info.flags = pVar ? pVar->as<ComponentInfoFlags>() : ComponentInfoFlags();

  pDesc->baseClass = typeDesc["super"].asSharedString();

  pDesc->pInit = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pCreateInstance = [](const ComponentDesc *_pType, Kernel *_pKernel, SharedString _uid, Variant::VarMap initParams) -> ComponentRef {
    MutableString128 t(Format, "New (From VarMap): {0} - {1}", _pType->info.identifier, _uid);
    _pKernel->LogDebug(4, t);
    const DynamicComponentDesc *pDesc = (const DynamicComponentDesc*)_pType;
    DynamicComponentRef spInstance = pDesc->newInstance(KernelRef(_pKernel), initParams);
    ComponentRef spC = _pKernel->CreateGlue(pDesc->baseClass, _pType, _uid, spInstance, initParams);
    spInstance->AttachToGlue(spC.ptr(), initParams);
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
    pDesc->propertyTree.insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : CreateHelper<_ComponentType>::GetMethods())
    pDesc->methodTree.insert(m.id, { m, m.pMethod });
  for (auto &e : CreateHelper<_ComponentType>::GetEvents())
    pDesc->eventTree.insert(e.id, { e, e.pSubscribe });
  for (auto &f : CreateHelper<_ComponentType>::GetStaticFuncs())
    pDesc->staticFuncTree.insert(f.id, { f, (void*)f.pCall });
*/

  // setup the super class and populate from its meta
  pDesc->pSuperDesc = GetComponentDesc(pDesc->baseClass);
  EPTHROW_IF(!pDesc->pSuperDesc, Result::InvalidType, "Base Component '{0}' not registered", pDesc->baseClass);
  pDesc->PopulateFromDesc((ComponentDescInl*)pDesc->pSuperDesc);

  return RegisterComponentType(pDesc);
}

void KernelImpl::RegisterGlueType(String name, CreateGlueFunc *pCreateFunc)
{
  glueRegistry.insert(name, pCreateFunc);
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
  ComponentType *pCT = componentRegistry.get(id);
  if (!pCT)
    return nullptr;
  return pCT->pDesc;
}

ComponentRef KernelImpl::CreateComponent(String typeId, Variant::VarMap initParams)
{
  ComponentType *_pType = componentRegistry.get(typeId);
  EPASSERT_THROW(_pType, Result::InvalidArgument, "Unknown component type {0}", typeId);
  EPTHROW_IF(_pType->pDesc->info.flags & ComponentInfoFlags::Abstract, Result::InvalidType, "Cannot create component of abstract type '{0}'", typeId);

  try
  {
    const ComponentDescInl *pDesc = _pType->pDesc;

    // TODO: should we have a better uid generator than this?
    MutableString64 newUid(Concat, pDesc->info.identifier, _pType->createCount++);

    // attempt to create an instance
    ComponentRef spComponent(pDesc->pCreateInstance(pDesc, pInstance, newUid, initParams));

    // add to the component registry
    instanceRegistry.insert(spComponent->uid, spComponent.ptr());

    // TODO: inform partner kernels that I created a component
    //...

    return spComponent;
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
  CreateGlueFunc **ppCreate = glueRegistry.get(typeId);
  EPTHROW_IF_NULL(ppCreate, Result::InvalidType, "No glue type {0}", typeId);
  return (*ppCreate)(pInstance, _pType, _uid, spInstance, initParams);
}

void KernelImpl::DestroyComponent(Component *_pInstance)
{
  if (_pInstance->name)
    namedInstanceRegistry.remove(_pInstance->name);
  instanceRegistry.remove(_pInstance->uid);

  // TODO: inform partners that I destroyed a component
  //...
}

ComponentRef KernelImpl::FindComponent(String _name) const
{
  if (_name.empty() || _name[0] == '$' || _name[0] == '#')
    return nullptr;
  if (_name[0] == '@')
    _name.popFront();
  Component * const * ppComponent = namedInstanceRegistry.get(_name);
  if (!ppComponent)
    ppComponent = instanceRegistry.get(_name);
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
  Result result = epHAL_InitRender();
  EPASSERT_THROW(result == Result::Success, result, "epHAL_InitRender() Failed");
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
    extensionsRegistry.insert(e, pDesc);
}

DataSourceRef KernelImpl::CreateDataSourceFromExtension(String ext, Variant::VarMap initParams)
{
  const ComponentDesc **ppDesc = extensionsRegistry.get(ext);
  EPASSERT_THROW(ppDesc, Result::Failure, "No datasource for extension {0}", ext);

  return component_cast<DataSource>(CreateComponent((*ppDesc)->info.identifier, initParams));
}

SharedPtr<Renderer> KernelImpl::GetRenderer() const
{
  return spRenderer;
}

CommandManagerRef KernelImpl::GetCommandManager() const
{
  return spCommandManager;
}

SettingsRef KernelImpl::GetSettings() const
{
  return spSettings;
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
