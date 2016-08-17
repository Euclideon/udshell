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

Array<const PropertyInfo> Kernel::getProperties() const
{
  return Array<const PropertyInfo>{
    EP_MAKE_PROPERTY_RO("resourceManager", getResourceManager, "Resource manager", nullptr, 0),
    EP_MAKE_PROPERTY_RO("commandManager", getCommandManager, "Command manager", nullptr, 0),
    EP_MAKE_PROPERTY_RO("stdOutBroadcaster", getStdOutBroadcaster, "stdout broadcaster", nullptr, 0),
    EP_MAKE_PROPERTY_RO("stdErrBroadcaster", getStdErrBroadcaster, "stderr broadcaster", nullptr, 0),
  };
}
Array<const MethodInfo> Kernel::getMethods() const
{
  return Array<const MethodInfo>{
    EP_MAKE_METHOD(exec, "Execute Lua script")
  };
}
Array<const EventInfo> Kernel::getEvents() const
{
  return Array<const EventInfo>{
    EP_MAKE_EVENT(updatePulse, "Periodic update signal")
  };
}
Array<const StaticFuncInfo> Kernel::getStaticFuncs() const
{
  return Array<const StaticFuncInfo>{
    EP_MAKE_STATICFUNC(getEnvironmentVar, "Get an environment variable"),
    EP_MAKE_STATICFUNC(setEnvironmentVar, "Set an environment variable")
  };
}


ComponentDescInl *Kernel::makeKernelDescriptor(ComponentDescInl *pType)
{
  ComponentDescInl *pDesc = epNew(ComponentDescInl);
  EPTHROW_IF_NULL(pDesc, Result::AllocFailure, "Memory allocation failed");

  pDesc->info = Kernel::componentInfo();
  pDesc->info.flags = ComponentInfoFlags::Unregistered;
  pDesc->baseClass = Component::componentID();

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pSuperDesc = nullptr;

  // build search trees
  for (auto &p : Kernel::getPropertiesImpl())
    pDesc->propertyTree.insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : Kernel::getMethodsImpl())
    pDesc->methodTree.insert(m.id, { m, m.pMethod });
  for (auto &e : Kernel::getEventsImpl())
    pDesc->eventTree.insert(e.id, { e, e.pSubscribe });
  for (auto &f : Kernel::getStaticFuncsImpl())
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
  : Component(Kernel::makeKernelDescriptor(_pType), nullptr, "ep.Kernel0", commandLine)
{
  // alloc impl
  pImpl = UniquePtr<Impl>(epNew(KernelImpl, this, commandLine));
  getImpl()->StartInit(commandLine);
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

// HACK !!! (GCC and Clang)
// For the implementation of epInternalInit defined in globalinitialisers to override
// the weak version in epplatform.cpp at least one symbol from that file must
// be referenced externally.  This has been implemented in below inside
// createInstance().
namespace internal { void *getStaticImplRegistry(); }

Kernel* Kernel::createInstance(Variant::VarMap commandLine, int renderThreadCount)
{
  // HACK: create the KernelImplStatic instance here!
  ((HashMap<SharedString, UniquePtr<RefCounted>>*)internal::getStaticImplRegistry())->insert(componentID(), UniquePtr<KernelImplStatic>::create());

  // set $(AppPath) to argv[0]
  String exe = commandLine[0].asString();
#if defined(EP_WINDOWS)
  Kernel::setEnvironmentVar("AppPath", exe.getLeftAtLast('\\', true));
#else
  Kernel::setEnvironmentVar("AppPath", exe.getLeftAtLast('/', true));
#endif

  if (!commandLine.get("renderThreadCount"))
  {
    auto map = commandLine.clone();
    map.insert("renderThreadCount", renderThreadCount);
    commandLine = std::move(map);
  }

  return createInstanceInternal(commandLine);
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
  , commandLineArgs(initParams)
{
  s_pInstance->pKernelInstance = pInstance;
}

void KernelImpl::StartInit(Variant::VarMap initParams)
{
  // init the kernel
  epscope(fail) { DebugFormat("Error creating Kernel\n"); };

  renderThreadCount = initParams["renderThreadCount"].as<int>();

  // register global environment vars
  WrangleEnvironmentVariables();

  // register the base Component type
  pInstance->registerComponentType<Component, ComponentImpl, ComponentGlue>();

  // HACK: update the descriptor with the base class (bootup chicken/egg)
  const ComponentDescInl *pComponentBase = componentRegistry.get(Component::componentID())->pDesc;
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
  pInstance->Component::pImpl = pInstance->Component::createImpl(initParams);

  // register all the builtin component types
  pInstance->registerComponentType<DataSource, DataSourceImpl>();
  pInstance->registerComponentType<Broadcaster, BroadcasterImpl>();
  pInstance->registerComponentType<Stream, StreamImpl>();
  pInstance->registerComponentType<File, FileImpl, void, FileImplStatic>();
  pInstance->registerComponentType<StdIOStream>();
  pInstance->registerComponentType<MemStream, MemStreamImpl>();
  pInstance->registerComponentType<Regex, RegexImpl>();
  pInstance->registerComponentType<Logger>();
  pInstance->registerComponentType<PluginManager>();
  pInstance->registerComponentType<PluginLoader>();
  pInstance->registerComponentType<NativePluginLoader>();
  pInstance->registerComponentType<ResourceManager, ResourceManagerImpl>();
  pInstance->registerComponentType<CommandManager, CommandManagerImpl>();
  pInstance->registerComponentType<Project, ProjectImpl>();
  pInstance->registerComponentType<Timer, TimerImpl>();
  pInstance->registerComponentType<Settings, SettingsImpl>();
  pInstance->registerComponentType<Lua>();
  pInstance->registerComponentType<View, ViewImpl>();
  pInstance->registerComponentType<Activity, ActivityImpl>();
  pInstance->registerComponentType<Console>();
  pInstance->registerComponentType<PrimitiveGenerator, PrimitiveGeneratorImpl, void, PrimitiveGeneratorImplStatic>();

  // resources
  pInstance->registerComponentType<Resource, ResourceImpl>();
  pInstance->registerComponentType<Buffer, BufferImpl>();
  pInstance->registerComponentType<ArrayBuffer, ArrayBufferImpl>();
  pInstance->registerComponentType<UDModel, UDModelImpl>();
  pInstance->registerComponentType<Shader, ShaderImpl>();
  pInstance->registerComponentType<Material, MaterialImpl>();
  pInstance->registerComponentType<Model, ModelImpl>();
  pInstance->registerComponentType<Text, TextImpl, void, TextImplStatic>();
  pInstance->registerComponentType<Menu, MenuImpl>();
  pInstance->registerComponentType<KVPStore>();
  pInstance->registerComponentType<Metadata, MetadataImpl>();
  pInstance->registerComponentType<Scene, SceneImpl>();

  // nodes
  pInstance->registerComponentType<Node, NodeImpl>();
  pInstance->registerComponentType<SceneNode, SceneImpl>();
  pInstance->registerComponentType<Camera, CameraImpl>();
  pInstance->registerComponentType<SimpleCamera, SimpleCameraImpl>();
  pInstance->registerComponentType<GeomNode, GeomNodeImpl>();
  pInstance->registerComponentType<UDNode, UDNodeImpl>();

  // data sources
  pInstance->registerComponentType<ImageSource>();
  pInstance->registerComponentType<GeomSource>();
  pInstance->registerComponentType<UDSource>();

  // dynamic components
  pInstance->registerComponentType<DynamicComponent>();
  pInstance->registerComponentType<VarComponent>();

  // init the HAL
  EPTHROW_RESULT(epHAL_Init(), "epHAL_Init() failed");

  // create logger and default streams
  spLogger = pInstance->createComponent<Logger>();
  spLogger->disableCategory(LogCategories::Trace);

  try
  {
    StreamRef spDebugFile = pInstance->createComponent<File>({ { "name", "logfile" }, { "path", "epKernel.log" }, { "flags", FileOpenFlags::Append | FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Text } });
    spLogger->addStream(spDebugFile);
    spDebugFile->writeLn("\n*** Logging started ***");
  }
  catch (...) {}

#if EP_DEBUG
  StreamRef spStdIOStream = pInstance->createComponent<StdIOStream>({ { "output", StdIOStreamOutputs::StdDbg }, {"name", "debugout"} });
  spLogger->addStream(spStdIOStream);
#endif

  // resource manager
  spResourceManager = pInstance->createComponent<ResourceManager>({ { "name", "resourcemanager" } });

  // command manager
  spCommandManager = pInstance->createComponent<CommandManager>({ { "name", "commandmanager" } });

  // settings
  spSettings = pInstance->createComponent<Settings>({ { "name", "settings" }, { "src", "settings.epset" } });

  // plugin manager
  spPluginManager = pInstance->createComponent<PluginManager>({ { "name", "pluginmanager" } });
  spPluginManager->registerPluginLoader(pInstance->createComponent<NativePluginLoader>());

  // Init capture and broadcast of stdout/stderr
  spStdOutBC = pInstance->createComponent<Broadcaster>({ { "name", "stdoutbc" } });
  stdOutCapture = epNew(StdCapture, stdout);
  epscope(fail) { epDelete(stdOutCapture); };
  spStdErrBC = pInstance->createComponent<Broadcaster>({ { "name", "stderrbc" } });
  stdErrCapture = epNew(StdCapture, stderr);
  epscope(fail) { epDelete(stdErrCapture); };

  // create lua VM
  spLua = pInstance->createComponent<Lua>();

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
  {
    // TODO: Crash handler?
    if (!sendMessage("$register", "#", "register", nullptr))
    {
      pInstance->onFatal("Fatal error encountered during application register phase.\nSee epKernel.log for details.\n\nExiting...");
      pInstance->quit();
    }
  }

  // load the plugins...
  Array<const String> pluginPaths;

  // search env vars for extra plugin paths
  SharedString pluginPathsVar = Kernel::getEnvironmentVar("PluginDirs");
#if defined(EP_WINDOWS)
  String delimiters = ";";
#else
  String delimiters = ";:";
#endif
  pluginPathsVar.tokenise([&](String token, size_t) {
    pluginPaths.pushBack(token);
  }, delimiters);

  // search global settings for extra plugin paths
  //...

  // default search paths have lower precedence
  pluginPaths.concat(Slice<const String>{
    "bin/plugins", // *relative path* used during dev
#if defined(EP_LINUX)
    "$(HOME)/.local/share/Euclideon/plugins",
#endif
    "$(AppPath)plugins",
#if defined(EP_LINUX)
    "/usr/local/share/Euclideon/plugins",
    "/usr/share/Euclideon/plugins"
#endif
  });

  LoadAllPlugins(pluginPaths);

  // make the kernel timers
  spStreamerTimer = pInstance->createComponent<Timer>({ { "interval", 0.033 } });
  spStreamerTimer->elapsed.subscribe(FastDelegate<void()>(this, &KernelImpl::StreamerUpdate));

  spUpdateTimer = pInstance->createComponent<Timer>({ { "interval", 0.016 } });
  spUpdateTimer->elapsed.subscribe(FastDelegate<void()>(this, &KernelImpl::Update));

  // call application init
  if (HasMessageHandler("init"))
  {
    // TODO: Crash handler?
    if (!sendMessage("$init", "#", "init", commandLineArgs))
    {
      pInstance->onFatal("Fatal error encountered during application init phase.\nSee epKernel.log for details.\n\nExiting...");
      pInstance->quit();
    }
  }
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
      DebugFormat("Unfreed Component: {0} ({1}) refCount {2} \n", c.key, c.value->getName(), c.value->use_count());
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
    spStreamerTimer->elapsed.unsubscribe(FastDelegate<void()>(this, &KernelImpl::StreamerUpdate));
  if (spUpdateTimer)
    spUpdateTimer->elapsed.unsubscribe(FastDelegate<void()>(this, &KernelImpl::Update));

  // call application deinit
  if (HasMessageHandler("deinit"))
    sendMessage("$deinit", "#", "deinit", nullptr);

  pInstance->setFocusView(nullptr);

  spUpdateTimer = nullptr;
  spStreamerTimer = nullptr;

  spPluginManager = nullptr;

  spRenderer = nullptr;
}

void KernelImpl::WrangleEnvironmentVariables()
{
  // TODO: ...
}

Array<SharedString> KernelImpl::ScanPluginFolder(String folderPath, Slice<const String> extFilter)
{
  EPFindData findData;
  EPFind find;
  Array<SharedString> pluginFilenames;

  SharedString path = Kernel::resolveString(folderPath);

  if (!HalDirectory_FindFirst(&find, path.toStringz(), &findData))
    return nullptr;
  do
  {
    if (findData.attributes & EPFA_Directory)
    {
      MutableString<260> childFolderPath(Format, "{0}/{1}", path, String((const char*)findData.pFilename));

      Array<SharedString> childNames = ScanPluginFolder(childFolderPath, extFilter);
      for (SharedString &cName : childNames)
        pluginFilenames.pushBack(std::move(cName));
    }
    else
    {
      bool valid = true;
      MutableString<260> filename(Format, "{0}/{1}", path, String((const char*)findData.pFilename));
      for (auto &ext : extFilter)
      {
        valid = (filename.endsWithIC(ext));
        if (valid)
          break;
      }
      if (valid)
      {
        pInstance->logInfo(2, "  Found {0}", filename);
        pluginFilenames.pushBack(filename);
      }
    }
  } while (HalDirectory_FindNext(&find, &findData));

  HalDirectory_FindClose(&find);

  return pluginFilenames;
}

void KernelImpl::LoadAllPlugins(Slice<const String> folderPaths)
{
  for (auto path : folderPaths)
  {
    pInstance->logInfo(2, "Scanning {0} for plugins...", path);
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
      if (spPluginManager->loadPlugin(filename))
      {
        pInstance->logInfo(2, "Loaded plugin {0}", filename);
        filename = nullptr;
        --numRemaining;
      }
    }
  } while (numRemaining && numRemaining < lastTry);

  // output a warning if any plugins could not be loaded
  for (auto &filename : files)
  {
    if (filename)
      logWarning(2, "Could not load plugin '{0}'", filename);
  }
}

void KernelImpl::Update()
{
  static uint64_t last = udPerfCounterStart();
  uint64_t now = udPerfCounterStart();
  double sec = (double)udPerfCounterMilliseconds(last, now) / 1000.0;
  last = now;

  RelayStdIO();

  pInstance->updatePulse.signal(sec);
}

void KernelImpl::RelayStdIO()
{
  if (stdOutCapture)
  {
    String str = stdOutCapture->getCapture();
    if (!str.empty())
      spStdOutBC->write(str);
  }
  if (stdErrCapture)
  {
    String str = stdErrCapture->getCapture();
    if (!str.empty())
      spStdErrBC->write(str);
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
    SceneRef spScene = spFocusView->getScene();
    if (spScene)
      spScene->makeDirty();
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

bool KernelImpl::sendMessage(String target, String sender, String message, const Variant &data)
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
        spComponent->receiveMessage(message, sender, data);
      } catch (std::exception &e) {
        logError("Message Handler {0} failed: {1}", target, e.what());
        return false;
      } catch (...) {
        logError("Message Handler {0} failed: C++ exception", target);
        return false;
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
        logError("Message Handler {0} failed: {1}", target, e.what());
        return false;
      } catch (...) {
        logError("Message Handler {0} failed: C++ exception", target);
        return false;
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
        logError("Message Handler {0} failed: {1}", target, e.what());
        return false;
      } catch (...) {
        logError("Message Handler {0} failed: C++ exception", target);
        return false;
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
  return true;
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

//  pDesc->info.displayName = typeDesc["name"].asSharedString(); // TODO: add this back at some point?
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
    _pKernel->logDebug(4, t);
    const DynamicComponentDesc *pDesc = (const DynamicComponentDesc*)_pType;
    DynamicComponentRef spInstance = pDesc->newInstance(KernelRef(_pKernel), initParams);
    ComponentRef spC = _pKernel->createGlue(pDesc->baseClass, _pType, _uid, spInstance, initParams);
    spInstance->attachToGlue(spC.get(), initParams);
    spC->pUserData = spInstance->getUserData();
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
    instanceRegistry.insert(spComponent->uid, spComponent.get());

    // TODO: inform partner kernels that I created a component
    //...

    return spComponent;
  }
  catch (std::exception &e)
  {
    logWarning(3, "Create component failed: {0}", String(e.what()));
    throw;
  }
  catch (...)
  {
    logWarning(3, "Create component failed!");
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
  spLua->execute(code);
}

void KernelImpl::Log(int kind, int level, String text, String component) const
{
  if (spLogger)
    spLogger->log(level, text, (LogCategories)kind, component);
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

void KernelImplStatic::SetEnvironmentVar(String name, String value)
{
#if defined(EP_WINDOWS)
  _putenv_s(name.toStringz(), value.toStringz());
#else
  setenv(name.toStringz(), value.toStringz(), 1);
#endif
}

MutableString<0> KernelImplStatic::GetEnvironmentVar(String name)
{
#if defined(EP_WINDOWS)
  MutableString<0> r;
  auto sz = name.toStringz();
  size_t size;
  getenv_s(&size, nullptr, 0, sz);
  if (size)
  {
    r.reserve(size);
    getenv_s(&size, r.ptr, size, sz);
    r.length = size-1;
  }
  return r;
#else
  return getenv(name.toStringz());
#endif
}

MutableString<0> KernelImplStatic::ResolveString(String string, bool bRecursive)
{
  // TODO: do this loop in blocks rather than one byte at a time!
  MutableString<0> r(Reserve, string.length);
  for (size_t i = 0; i < string.length; ++i)
  {
    if (string[i] == '$' && string.length > i+1 && string[i+1] == '$')
    {
      ++i;
      r.pushBack(string[i]);
    }
    else if (string[i] == '$' && string.length > i+2 && string[i+1] == '(')
    {
      size_t end = i + 2;
      while (end < string.length && string[end] != ')')
        ++end;
      String var = string.slice(i+2, end);
      auto val = GetEnvironmentVar(var);
      if (val)
      {
        if (bRecursive)
          val = ResolveString(val, true);
        r.append(val);
      }
      i = end;
    }
    else
      r.pushBack(string[i]);
  }
  return r;
}

} // namespace ep

#if __EP_MEMORY_DEBUG__
#if defined(EP_WINDOWS)
namespace ep {
  namespace internal {

    int reportingHook(int reportType, char* userMessage, int* retVal)
    {
      static bool filter = true;
      static int debugMsgCount = 3;
      static int leakCount = 0;

      if (strcmp(userMessage, "Object dump complete.\n") == 0)
        filter = false;

      if (filter)
      {
        // Debug messages from our program should consist of 4 parts :
        // File (line) | AllocID | Block Descriptor | Memory Data
        if (!strstr(userMessage, ") : "))
        {
          ++debugMsgCount;
        }
        else
        {
          if (leakCount == 0)
            OutputDebugStringA("Detected memory leaks!\nDumping objects ->\n");
          debugMsgCount = 0;
          ++leakCount;
        }
        // Filter the output if it's not from our program
        return (debugMsgCount > 3);
      }

      return (leakCount == 0);
    }

  } // namespace internal
} // namespace ep
#endif  // defined(EP_WINDOWS)

# if defined (epInitMemoryTracking)
#   undef epInitMemoryTracking
# endif // epInitMemoryTracking
void epInitMemoryTracking()
{
#if defined(EP_WINDOWS)
  const wchar_t *pFilename = L"MemoryReport_"
#if EP_DEBUG
    "Debug_"
#else
    "Release_"
#endif // EP_DEBUG
#if defined(EP_ARCH_X64)
    "x64"
#elif defined(EP_ARCH_X86)
    "x86"
#else
#   error "Couldn't detect target architecture"
#endif // defined (EP_ARCH_X64)
    ".txt";

  HANDLE hCrtWarnReport = CreateFileW(pFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hCrtWarnReport == INVALID_HANDLE_VALUE) OutputDebugStringA("Error creating CrtWarnReport.txt\n");

  errno = 0;
  int warnMode = _CrtSetReportMode(_CRT_WARN, _CRTDBG_REPORT_MODE);
  _CrtSetReportMode(_CRT_WARN, warnMode | _CRTDBG_MODE_FILE);
  if (errno == EINVAL) OutputDebugStringA("Error calling _CrtSetReportMode() warnings\n");

  errno = 0;
  _CrtSetReportFile(_CRT_WARN, hCrtWarnReport);
  if (errno == EINVAL)OutputDebugStringA("Error calling _CrtSetReportFile() warnings\n");
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

  //change the report function to only report memory leaks from program code
  _CrtSetReportHook(ep::internal::reportingHook);
#endif
}
#endif // __EP_MEMORY_DEBUG__
