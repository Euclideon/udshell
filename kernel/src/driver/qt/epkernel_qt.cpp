#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_QT

#include "kernelimpl.h"
#include "components/viewimpl.h"
#include "components/glue/componentglue.h"
#include "components/pluginmanager.h"

#include "driver/qt/epkernel_qt.h"
#include "driver/qt/util/qmlbindings_qt.h"
#include "driver/qt/util/typeconvert_qt.h"
#include "driver/qt/ui/renderview_qt.h"
#include "driver/qt/ui/window_qt.h"
#include "driver/qt/components/qmlpluginloader_qt.h"
#include "driver/qt/components/qobjectcomponent_qt.h"
#include "driver/qt/components/windowimpl_qt.h"
#include "driver/qt/components/uicomponentimpl_qt.h"
#include "driver/qt/components/viewportimpl_qt.h"

#include <QSemaphore>
#include <QDirIterator>


// Init the kernel's qrc file resources - this has to happen from the global namespace
inline void InitResources() { Q_INIT_RESOURCE(kernel); }

namespace qt {

static QObject *QtKernelQmlSingletonProvider(QQmlEngine *pEngine, QJSEngine *pScriptEngine)
{
  Q_UNUSED(pEngine);
  Q_UNUSED(pScriptEngine);

  // NOTE: QML takes ownership of this object
  return new QtKernelQml((QtKernel*)QtApplication::Kernel());
}

// custom kernel event
class KernelEvent : public QEvent
{
public:
  KernelEvent(const FastDelegateMemento &mem) : QEvent(type()), m(mem) {}
  virtual ~KernelEvent() {}

  static QEvent::Type type()
  {
    if (eventType == QEvent::None)
      eventType = (QEvent::Type)QEvent::registerEventType();
    return eventType;
  }

  FastDelegateMemento m;

private:
  static QEvent::Type eventType;
};

QEvent::Type KernelEvent::eventType = QEvent::None;

// custom kernel event with semaphore
class KernelSyncEvent : public KernelEvent
{
public:
  KernelSyncEvent(const FastDelegateMemento &mem, QSemaphore *pS) : KernelEvent(mem), pSem(pS) {}
  virtual ~KernelSyncEvent() { if (pSem) pSem->release(); }

  QSemaphore *pSem;
};


using ep::ComponentDescInl;
using ep::Variant;
using ep::SharedPtr;
using ep::String;
using ep::SharedString;


/** QtKernel *********************************************/

ComponentDescInl *QtKernel::MakeKernelDescriptor()
{
  ComponentDescInl *pDesc = epNew(ComponentDescInl);
  EPTHROW_IF_NULL(pDesc, epR_AllocFailure, "Memory allocation failed");

  pDesc->info = QtKernel::ComponentInfo();
  pDesc->info.flags = ep::ComponentInfoFlags::Unregistered;
  pDesc->baseClass = Kernel::ComponentID();

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pSuperDesc = nullptr;

  // build search trees
  for (auto &p : CreateHelper<QtKernel>::GetProperties())
    pDesc->propertyTree.Insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : CreateHelper<QtKernel>::GetMethods())
    pDesc->methodTree.Insert(m.id, { m, m.pMethod });
  for (auto &e : CreateHelper<QtKernel>::GetEvents())
    pDesc->eventTree.Insert(e.id, { e, e.pSubscribe });
  for (auto &f : CreateHelper<QtKernel>::GetStaticFuncs())
    pDesc->staticFuncTree.Insert(f.id, { f, (void*)f.pCall });

  return pDesc;
}

// ---------------------------------------------------------------------------------------
QtKernel::QtKernel(Variant::VarMap commandLine)
  : ep::Kernel(QtKernel::MakeKernelDescriptor(), commandLine)
{
  // convert Variant::VarMap back into a string list for Qt
  // NOTE: this assumes that the char* list referred to by commandLine will remain valid for the entire lifetime of the Kernel
  // NOTE: the state of our argv may be changed by Qt as it removes args that it recognises
  for (auto arg : commandLine)
  {
    if(arg.key.is(Variant::Type::Int))
      cmdArgs.pushBack(arg.value.asSharedString());
  }
  for (auto &arg : cmdArgs)
    cmdArgv.pushBack(arg.ptr);
  cmdArgc = (int)cmdArgv.length;

  // register Qt specific components
  EPTHROW_IF_NULL(RegisterComponentType<QmlPluginLoader>(), epR_Failure, "Unable to register QmlPluginLoader");
  EPTHROW_IF_NULL(RegisterComponentType<QObjectComponent>(), epR_Failure, "Unable to register QtComponent");
  EPTHROW_IF_NULL((RegisterComponentType<ep::UIComponent, QtUIComponentImpl, UIComponentGlue>()), epR_Failure, "Unable to register UI Component");
  EPTHROW_IF_NULL((RegisterComponentType<ep::Window, QtWindowImpl, WindowGlue>()), epR_Failure, "Unable to register Window component");
  EPTHROW_IF_NULL((RegisterComponentType<ep::Viewport, QtViewportImpl, ViewportGlue>()), epR_Failure, "Unable to register Viewport UI Component");

  // create and register the qml loader
  spQmlPluginLoader = CreateComponent<QmlPluginLoader>();
  GetImpl()->spPluginManager->RegisterPluginLoader(spQmlPluginLoader);

  // create our QApplication
  pApplication = new QtApplication(this, cmdArgc, (char**)cmdArgv.ptr);
  EPTHROW_IF_NULL(pApplication, epR_Failure, "Unable create QtApplication");
  epscope(fail) { delete pApplication; };

  // make sure we cleanup the kernel when we're about to quit
  pMediator = new QtKernelMediator(this);
  epscope(fail) { delete pMediator; };
  EPTHROW_IF(!QObject::connect(pApplication, &QCoreApplication::aboutToQuit, pMediator, &QtKernelMediator::OnAppQuit), epR_Failure, "Failed to create Qt connection");

  // create the qml engine
  pQmlEngine = new QQmlEngine;
  EPTHROW_IF_NULL(pQmlEngine, epR_Failure, "Unable create QQmlEngine");
  epscope(fail) { delete pQmlEngine; };

  // register our internal qml types
  EPTHROW_IF(qmlRegisterType<QtRenderView>("epKernel", 0, 1, "EPRenderView") == -1, epR_Failure, "qmlRegisterType<QtRenderView> Failed");
  EPTHROW_IF(qmlRegisterType<QtEPComponent>() == -1, epR_Failure, "qmlRegisterType<QtEPComponent> Failed");
  EPTHROW_IF(qRegisterMetaType<QtFocusManager*>("QtFocusManager*") == -1, epR_Failure, "qRegisterMetaType<QtFocusManager *> Failed");
  EPTHROW_IF(qmlRegisterSingletonType<QtKernelQml>("epKernel", 0, 1, "EPKernel", QtKernelQmlSingletonProvider) == -1, epR_Failure, "qmlRegisterSingletonType<QtKernelQml> Failed");

  // load in the kernel qml resources
  InitResources();
  RegisterQmlComponents(":/kernel");

  // modify our surface format to support opengl debug logging
  // TODO: set gl version based on property settings?
  // TODO: control setting a debug context based on command line switches?
  mainSurfaceFormat.setOption(QSurfaceFormat::DebugContext);
  QSurfaceFormat::setDefaultFormat(mainSurfaceFormat);

  // create focus manager;
  pFocusManager = new QtFocusManager();

  // create the splash screen
  QQmlComponent component(pQmlEngine, QUrl("qrc:/kernel/splashscreen.qml"));
  QObject *pObject = component.create();
  epscope(fail) { delete pObject; };
  pSplashScreen = qobject_cast<QQuickWindow*>(pObject);
  if (!pSplashScreen)
  {
    // TODO: better error information/handling
    foreach(const QQmlError &error, component.errors())
      LogError(SharedString::concat("QML Error: ", error.toString().toUtf8().data()));

    EPTHROW_ERROR(epR_Failure, "Error creating Splash Screen");
  }

  // defer the heavier init stuff and app specific init to after Qt hits the event loop
  // we'll hook into the splash screen to do this
  EPTHROW_IF(!QObject::connect(pSplashScreen, &QQuickWindow::afterRendering, pMediator, &QtKernelMediator::OnFirstRender, Qt::DirectConnection),
    epR_Failure, "Failed to create Qt connection for Splash Screen");
}

// ---------------------------------------------------------------------------------------
QtKernel::~QtKernel()
{
  delete pMediator;
  delete pApplication;
}

// ---------------------------------------------------------------------------------------
void QtKernel::RunMainLoop()
{
  // run the Qt event loop - this may never return
  EPTHROW_IF(pApplication->exec() != 0, epR_Failure, "Application was not shutdown from a call to quit()");
}

// ---------------------------------------------------------------------------------------
void QtKernel::Quit()
{
  pTopLevelWindow->close();
}

// ---------------------------------------------------------------------------------------
epResult QtKernel::RegisterWindow(QQuickWindow *pWindow)
{
  // sanity checks
  EPASSERT(pSplashScreen != nullptr, "No splash screen window set");
  EPASSERT(pMainThreadContext != nullptr, "No main thread GL context");
  EPASSERT(pGLDebugLogger != nullptr, "No GL debugger");

  // TODO: support adding multiple windows?
  EPASSERT(pTopLevelWindow.isNull(), "Multiple windows not supported - you must unregister your current window first");

  // TODO: maybe destroy this rather than keeping it around - check mem usage
  if (pSplashScreen->isVisible())
    pSplashScreen->hide();

  pGLDebugLogger->stopLogging();
  pMainThreadContext->doneCurrent();
  pTopLevelWindow = pWindow;

  // Hook up window signals
  QObject::connect(pTopLevelWindow.data(), &QQuickWindow::openglContextCreated, pMediator, &QtKernelMediator::OnGLContextCreated, Qt::DirectConnection);

  // install event filter - this will get automatically cleaned up when the top window is destroyed
  pTopLevelWindow->installEventFilter(new QtWindowEventFilter(pTopLevelWindow));

  pTopLevelWindow->show();
  pTopLevelWindow->raise();

  if (!pMainThreadContext->makeCurrent(pTopLevelWindow))
  {
    // TODO: error handle
    LogError("Error making main gl context current");
    return epR_Failure;
  }

  if (pGLDebugLogger->initialize())
    pGLDebugLogger->startLogging();

  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void QtKernel::UnregisterWindow(QQuickWindow *pWindow)
{
  // if we're unregistering the top level window then stop logging and switch the context
  // back to the splash screen since this is a surface that we know exists
  // this means we can log thru to final cleanup
  if (pTopLevelWindow.data() == pWindow)
  {
    // sanity checks
    EPASSERT(pSplashScreen != nullptr, "No splash screen window set");
    EPASSERT(pMainThreadContext != nullptr, "No main thread GL context");
    EPASSERT(pGLDebugLogger != nullptr, "No GL debugger");

    pGLDebugLogger->stopLogging();
    pMainThreadContext->makeCurrent(pSplashScreen);

    if (pGLDebugLogger->initialize())
      pGLDebugLogger->startLogging();
  }

  QObject::disconnect(pWindow, &QQuickWindow::openglContextCreated, pMediator, &QtKernelMediator::OnGLContextCreated);
}

// ---------------------------------------------------------------------------------------
void QtKernel::RegisterQmlComponent(String file)
{
  EPASSERT_THROW(!file.empty(), epR_InvalidArgument, "Must supply a valid file to register a QML component type");

  ep::Variant::VarMap typeDesc;
  try {
    typeDesc = QmlPluginLoader::ParseTypeDescriptor(this, file);
  }
  catch (ep::EPException &e) {
    EPTHROW_ERROR(epR_Failure, "Could not register QML file '{0}' as Component: \"{1}\"", file, e.what());
  }

  EPTHROW_IF(typeDesc.Empty(), epR_Failure, "Cannot register QML Component: File '{0}' does not contain valid type descriptor", file);
  RegisterQml(file, typeDesc);
}

// ---------------------------------------------------------------------------------------
void QtKernel::RegisterQmlComponents(String folderPath)
{
  EPASSERT_THROW(!folderPath.empty(), epR_InvalidArgument, "Must supply a valid folderPath to search for QML components");

  ep::Array<ep::SharedString> qmlFilenames;

  // if the path refers to the Qt resource system, then scan the resource file
  if (folderPath[0] == ':')
  {
    QDirIterator it(epToQString(folderPath), QDirIterator::Subdirectories);
    while (it.hasNext())
    {
      QString file = it.next();
      if (file.endsWith(QStringLiteral(".qml"), Qt::CaseInsensitive))
        qmlFilenames.pushBack(epFromQString(file));
    }
  }
  // otherwise assume it's a file system path and pass it via the plugin manager
  else
  {
     qmlFilenames = GetImpl()->ScanPluginFolder(folderPath, {".qml"});
  }

  pQmlEngine->addImportPath(epToQString(folderPath));
  GetImpl()->LoadPlugins(qmlFilenames);
}

// ---------------------------------------------------------------------------------------
void QtKernel::RegisterQml(ep::String file, ep::Variant::VarMap typeDesc)
{
  auto data = SharedPtr<QmlComponentData>::create(file, pQmlEngine, QQmlComponent::Asynchronous);
  Variant::VarMap typeInfo = {
    { "identifier", typeDesc["id"].asSharedString() },
    { "name", typeDesc["displayname"].asSharedString() },
    { "description", typeDesc["description"].asSharedString() },
    { "version", typeDesc["version"].as<int>() },
    { "flags", ep::ComponentInfoFlags::Unpopulated },
    { "super", typeDesc["super"].asSharedString() },
    { "userdata", (const SharedPtr<RefCounted>&)data },
    { "new", ep::DynamicComponentDesc::NewInstanceFunc(data.ptr(), &QmlComponentData::CreateComponent) }
  };

  GetImpl()->RegisterComponentType(typeInfo);
}

// ---------------------------------------------------------------------------------------
ep::ComponentRef QtKernel::CreateQmlComponent(String file, Variant::VarMap initParams)
{
  using namespace ep;
  EPASSERT_THROW(!file.empty(), epR_InvalidArgument, "Must supply a valid file to register a QML component type");

  // verify that the qml file has a type descriptor property
  Variant::VarMap typeDesc;
  try {
    typeDesc = QmlPluginLoader::ParseTypeDescriptor(this, file);
  }
  catch (ep::EPException &e) {
    EPTHROW_ERROR(epR_Failure, "Could not create QML Component from file '{0}': \"{1}\"", file, e.what());
  }

  EPTHROW_IF(typeDesc.Empty(), epR_Failure, "Cannot create QML Component: File '{0}' does not contain valid type descriptor", file);

  // locate the nominated super
  String superTypeId = typeDesc["super"].asString();
  const ComponentDescInl *pSuper = (const ComponentDescInl *)GetImpl()->GetComponentDesc(superTypeId);
  EPTHROW_IF(!pSuper, epR_InvalidType, "Base Component '{0}' not registered", superTypeId);

  // generate unregistered component descriptor
  DynamicComponentDesc *pDesc = epNew(DynamicComponentDesc);
  epscope(fail) { epDelete(pDesc); };

  pDesc->pSuperDesc = pSuper;
  pDesc->baseClass = pSuper->info.identifier;

  pDesc->info.identifier = typeDesc["id"].asSharedString();

  size_t offset = pDesc->info.identifier.findLast('.');
  EPTHROW_IF(offset == (size_t)-1, epR_InvalidArgument, "Component identifier {0} has no namespace. Use form: namespace.componentname", pDesc->info.identifier);

  pDesc->info.nameSpace = pDesc->info.identifier.slice(0, offset);
  pDesc->info.name = pDesc->info.identifier.slice(offset + 1, pDesc->info.identifier.length);

  pDesc->info.displayName = typeDesc["displayname"].asSharedString();
  pDesc->info.description = typeDesc["description"].asSharedString();
  pDesc->info.epVersion = pSuper->info.epVersion;
  pDesc->info.pluginVersion = typeDesc["version"].as<int>();
  pDesc->info.flags = ComponentInfoFlags::Unregistered | ComponentInfoFlags::Unpopulated;

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->userData = nullptr;
  pDesc->newInstance = nullptr;

  // TODO: should we have a better uid generator than this?
  static int serialNumber = 0;
  MutableString64 newUid(Concat, pDesc->info.identifier, "_", serialNumber++);

  // create the new component (glue and instance)
  pKernel->LogDebug(4, "New (Unregistered QML Component): {0} - {1}", pDesc->info.identifier, file);
  QmlComponentData data(file, pQmlEngine);
  QObjectComponentRef spInstance = shared_pointer_cast<QObjectComponent>(data.CreateComponent(KernelRef(this)));
  ComponentRef spC = CreateGlue(pDesc->baseClass, pDesc, newUid, spInstance, initParams);
  spInstance->AttachToGlue(spC.ptr());
  pDesc->PopulateFromDesc(pSuper);

  // add to the component registry
  GetImpl<KernelImpl>()->instanceRegistry.Insert(spC->uid, spC.ptr());

  return spC;
}

// ---------------------------------------------------------------------------------------
void QtKernel::FinishInit()
{
  epscope(fail) { LogError("Error initialising renderer");  pApplication->quit(); };

  EPASSERT(pSplashScreen != nullptr, "No splash screen window set");
  EPASSERT(pMainThreadContext == nullptr, "Main Thread context already exists");

  // create main opengl context
  pMainThreadContext = new QOpenGLContext();
  pMainThreadContext->setFormat(mainSurfaceFormat);
  IF_EPASSERT(bool succeed = )pMainThreadContext->create();
  EPASSERT(succeed, "Couldn't create render context!");

  // update the format based on what we actually got (since it may differ)
  mainSurfaceFormat = pMainThreadContext->format();

  LogDebug(2, "Created OpenGL context using version: {0}.{1} {2,?3}",
    mainSurfaceFormat.majorVersion(),
    mainSurfaceFormat.minorVersion(),
    (mainSurfaceFormat.profile() == QSurfaceFormat::CoreProfile ? "Core" : "Compatibility"),
    (mainSurfaceFormat.profile() != QSurfaceFormat::NoProfile));

  if (!pMainThreadContext->makeCurrent(pSplashScreen))
  {
    // TODO: handle error
    LogError("Error making main GL Context current");
    pApplication->quit();
  }

  // TODO: kill this in release builds?
  pGLDebugLogger = new QOpenGLDebugLogger();
  QObject::connect(pGLDebugLogger, &QOpenGLDebugLogger::messageLogged, pMediator, &QtKernelMediator::OnGLMessageLogged);

  if (pGLDebugLogger->initialize())
  {
    // enable synchronous if we need to preserve the gl debug logging order
    //pGLDebugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    pGLDebugLogger->startLogging();
  }

  // init the HAL's render system
  GetImpl()->InitRender();

  // app specific init
  Kernel::FinishInit();
}

// ---------------------------------------------------------------------------------------
void QtKernel::Shutdown()
{
  // shutdown the app then pump the message queue to flush jobs from UD render thread completion
  GetImpl()->Shutdown();
  pApplication->sendPostedEvents();
  pApplication->processEvents();

  delete pGLDebugLogger;
  delete pFocusManager;

  // delete the QML engine and then pump the message queue to ensure we don't have pending resource destruction
  pQmlEngine->clearComponentCache();
  pQmlEngine->collectGarbage();
  delete pQmlEngine;
  pApplication->sendPostedEvents(nullptr, QEvent::DeferredDelete);
  pApplication->processEvents();

  delete pSplashScreen;

  try
  {
    GetImpl()->DeinitRender();
  }
  catch (std::exception &e)
  {
    LogError("Error cleaning up renderer, DeinitRender failed: {0}", e.what());
  }
  catch (...)
  {
    LogError("Error cleaning up renderer, DeinitRender failed");
  }

  delete pMainThreadContext;

  spQmlPluginLoader = nullptr;
}

// ---------------------------------------------------------------------------------------
ep::ViewRef QtKernel::SetFocusView(ep::ViewRef spView)
{
  using namespace ep;
  KernelImpl *pKernelImpl = GetImpl();
  if (!spView && pKernelImpl->spFocusView)
    pKernelImpl->spFocusView->GetImpl<ViewImpl>()->SetLatestFrame(nullptr);

  ViewRef spOld = pKernelImpl->spFocusView;
  pKernelImpl->spFocusView = spView;
  return spOld;
}

// ---------------------------------------------------------------------------------------
void QtKernel::DispatchToMainThread(ep::MainThreadCallback callback)
{
  // if we're on the main thread just execute the callback now
  if (OnMainThread())
    callback();
  // otherwise jam it into the event queue
  else
    pMediator->PostEvent(new KernelEvent(callback.GetMemento()), Qt::NormalEventPriority);
}

// ---------------------------------------------------------------------------------------
void QtKernel::DispatchToMainThreadAndWait(ep::MainThreadCallback callback)
{
  // TODO: handle this gracefully? can we detect if the main thread is blocked??
  EPASSERT(!OnRenderThread(), "DispatchToMainThreadAndWait() should not be called on the Render Thread");

  // if we're on the main thread just execute the callback now
  if (OnMainThread())
  {
    callback();
  }
  // otherwise jam it into the event queue
  else
  {
    QSemaphore sem;
    pMediator->PostEvent(new qt::KernelSyncEvent(callback.GetMemento(), &sem), Qt::NormalEventPriority);
    sem.acquire();
  }
}


// ---------------------------------------------------------------------------------------
void QtKernelMediator::PostEvent(QEvent *pEvent, int priority)
{
  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  EPASSERT(pQtKernel->pApplication != nullptr, "QApplication doesn't exist");

  pQtKernel->pApplication->postEvent(this, pEvent, priority);
}

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void QtKernelMediator::OnGLContextCreated(QOpenGLContext *pContext)
{
  EPASSERT(pQtKernel->pMainThreadContext != nullptr, "Expected GL context");

  // we need to share our context with Qt and recreate
  pContext->setShareContext(pQtKernel->pMainThreadContext);
  IF_EPASSERT(bool succeed = ) pContext->create();

  // TODO: error handle
  EPASSERT(succeed, "Couldn't create shared render context!");
}

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void QtKernelMediator::OnFirstRender()
{
  // we only want this called on the first render cycle
  // this will be after the qt scenegraph and render thread has been created for the first window (splash screen)
  QObject::disconnect(pQtKernel->pSplashScreen, &QQuickWindow::afterRendering, this, &QtKernelMediator::OnFirstRender);

  // TODO: hook up render thread id stuff again
  pQtKernel->renderThreadId = QThread::currentThreadId();

  // dispatch init call to create our render context and resources on the main thread
  pQtKernel->DispatchToMainThread(MakeDelegate(pQtKernel, &QtKernel::FinishInit));
}

// ---------------------------------------------------------------------------------------
void QtKernelMediator::OnAppQuit()
{
  pQtKernel->Shutdown();
}

// ---------------------------------------------------------------------------------------
void QtKernelMediator::OnGLMessageLogged(const QOpenGLDebugMessage &debugMessage)
{
  // TODO: improve the formatting/verbosity of this
  pQtKernel->LogDebug(2, SharedString::concat("GL Message: ", debugMessage.message().toUtf8().data()));
}

// ---------------------------------------------------------------------------------------
void QtKernelMediator::customEvent(QEvent *pEvent)
{
  using namespace ep;
  if (pEvent->type() == KernelEvent::type())
  {
    MainThreadCallback d;
    d.SetMemento(static_cast<KernelEvent*>(pEvent)->m);
    size_t errorDepth = ErrorLevel();
    try
    {
      d();
      if (ErrorLevel() > errorDepth)
      {
        pQtKernel->LogError("Exception occurred in MainThreadCallback : {0}", GetError()->message);
        PopErrorToLevel(errorDepth);
      }
    }
    catch (std::exception &e)
    {
      pQtKernel->LogError("Exception occurred in MainThreadCallback : {0}", e.what());
      PopErrorToLevel(errorDepth);
    }
    catch (...)
    {
      pQtKernel->LogError("Exception occurred in MainThreadCallback : C++ Exception");
      PopErrorToLevel(errorDepth);
    }
  }
  else
  {
    pQtKernel->LogWarning(2, SharedString::concat("Unknown event received in Kernel: TYPE ", (int)pEvent->type()));
  }
}

} // namespace qt

namespace ep {

Kernel* Kernel::CreateInstanceInternal(Variant::VarMap commandLine)
{
  return KernelImpl::CreateComponentInstance<qt::QtKernel>(commandLine);
}

} // namespace ep

#else
EPEMPTYFILE
#endif
