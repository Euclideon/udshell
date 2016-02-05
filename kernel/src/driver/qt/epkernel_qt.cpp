#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_QT

#include "epkernel_qt.h"

#include "ui/renderview_qt.h"
#include "ui/window_qt.h"
#include "components/qobjectcomponent_qt.h"
#include "util/qmlbindings_qt.h"
#include "components/viewimpl.h"

#include <QSemaphore>


// Init the kernel's qrc file resources - this has to happen from the global namespace
inline void InitResources() { Q_INIT_RESOURCE(kernel); }

namespace qt
{

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


/** QtKernel *********************************************/

// ---------------------------------------------------------------------------------------
QtKernel::QtKernel(Slice<const KeyValuePair> commandLine)
  : QObject(0)
  , pApplication(nullptr)
  , pQmlEngine(nullptr)
  , pMainThreadContext(nullptr)
  , pGLDebugLogger(nullptr)
  , mainSurfaceFormat(QSurfaceFormat::defaultFormat())
  , pSplashScreen(nullptr)
  , pTopLevelWindow(nullptr)
  , mainThreadId(QThread::currentThreadId())
  , renderThreadId(nullptr)
{
  // convert Variant::VarMap back into a string list for Qt
  // NOTE: this assumes that the char* list referred to by commandLine will remain valid for the entire lifetime of the Kernel
  // NOTE: the state of our argv may be changed by Qt as it removes args that it recognises
  Array<char *, 1> args(Reserve, commandLine.length);
  argc = static_cast<int>(commandLine.length);
  for (int i = 0; i < argc; i++)
    args.pushBack(const_cast<char*>(commandLine[i].value.asString().ptr));

  argv = args.slice(0, argc);
}

// ---------------------------------------------------------------------------------------
QtKernel::~QtKernel()
{
  // pump the message queue - ours and any windows events
  pApplication->sendPostedEvents();
  pApplication->processEvents();

  delete pGLDebugLogger;
  delete pSplashScreen;
  delete pQmlEngine;

  try
  {
    DeinitRender();
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

  pApplication->deleteLater();
}

// ---------------------------------------------------------------------------------------
void QtKernel::InitInternal()
{
  LogTrace("QtKernel::InitInternal()");
  LogInfo(2, "Initialising epShell...");

  EPTHROW_IF_NULL(RegisterComponentType<QObjectComponent>(), epR_Failure, "Unable to register QtComponent");

  // create our qapplication
  pApplication = new QtApplication(this, argc, argv.ptr);
  EPTHROW_IF_NULL(pApplication, epR_Failure, "Unable create QtApplication");
  epscope(fail) { delete pApplication; };

  // make sure we cleanup the kernel when we're about to quit
  // TODO: Check this is correct! The call to ~QtApplication() should clean up any connections
  EPTHROW_IF(!QObject::connect(pApplication, &QCoreApplication::aboutToQuit, this, &QtKernel::OnAppQuit), epR_Failure, "Failed to create Qt connection");

  pQmlEngine = new QQmlEngine(this);
  EPTHROW_IF_NULL(pQmlEngine, epR_Failure, "Unable create QQmlEngine");
  epscope(fail) { delete pQmlEngine; };

  // register our internal qml types
  EPTHROW_IF(qmlRegisterType<QtRenderView>("epKernel", 0, 1, "EPRenderView") == -1, epR_Failure, "qmlRegisterType<QtRenderView> Failed");
  EPTHROW_IF(qmlRegisterType<QtEPComponent>() == -1, epR_Failure, "qmlRegisterType<QtEPComponent> Failed");
  EPTHROW_IF(qmlRegisterSingletonType<QtKernelQml>("epKernel", 0, 1, "EPKernel", QtKernelQmlSingletonProvider) == -1, epR_Failure, "qmlRegisterSingletonType<QtKernelQml> Failed");

  // Load in the qrc file
  InitResources();

  // modify our surface format to support opengl debug logging
  // TODO: set gl version based on property settings?
  // TODO: control setting a debug context based on command line switches?
  mainSurfaceFormat.setOption(QSurfaceFormat::DebugContext);
  QSurfaceFormat::setDefaultFormat(mainSurfaceFormat);

  // create the splash screen
  QQmlComponent component(pQmlEngine, QUrl("qrc:/kernel/splashscreen.qml"));
  QObject *object = component.create();
  pSplashScreen = qobject_cast<QQuickWindow*>(object);
  if (!pSplashScreen)
  {
    // TODO: better error information/handling
    foreach(const QQmlError &error, component.errors())
      LogError(SharedString::concat("QML Error: ", error.toString().toUtf8().data()));

    EPTHROW_ERROR(epR_Failure, "Error creating Splash Screen");
  }

  // defer the heavier init stuff and app specific init to after Qt hits the event loop
  // we'll hook into the splash screen to do this
  // TODO: Check this is correct! The call to ~QtApplication() should clean up any connections
  EPTHROW_IF(!QObject::connect(pSplashScreen, &QQuickWindow::afterRendering, this, &QtKernel::OnFirstRender, Qt::DirectConnection), epR_Failure, "Failed to create Qt connection for Splash Screen");
}

// ---------------------------------------------------------------------------------------
void QtKernel::RunMainLoop()
{
  LogTrace("QtKernel::RunMainLoop()");

  // run the Qt event loop - this may never return
  EPTHROW_IF(pApplication->exec() != 0, epR_Failure, "Application was not shutdown from a call to quit()");
}

// ---------------------------------------------------------------------------------------
void QtKernel::PostEvent(QEvent *pEvent, int priority)
{
  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  EPASSERT(pApplication != nullptr, "QApplication doesn't exist");

  pApplication->postEvent(this, pEvent, priority);
}

// ---------------------------------------------------------------------------------------
epResult QtKernel::RegisterWindow(QQuickWindow *pWindow)
{
  LogTrace("QtKernel::RegisterWindow()");

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
  QObject::connect(pTopLevelWindow.data(), &QQuickWindow::openglContextCreated, this, &QtKernel::OnGLContextCreated, Qt::DirectConnection);

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
  LogTrace("QtKernel::UnregisterWindow()");

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

  QObject::disconnect(pWindow, &QQuickWindow::openglContextCreated, this, &QtKernel::OnGLContextCreated);
}

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void QtKernel::OnGLContextCreated(QOpenGLContext *pContext)
{
  LogTrace("QtKernel::OnGLContextCreated()");

  EPASSERT(pMainThreadContext != nullptr, "Expected GL context");

  // we need to share our context with Qt and recreate
  pContext->setShareContext(pMainThreadContext);
  IF_EPASSERT(bool succeed =) pContext->create();

  // TODO: error handle
  EPASSERT(succeed, "Couldn't create shared render context!");
}

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void QtKernel::OnFirstRender()
{
  LogTrace("QtKernel::OnFirstRender()");

  // we only want this called on the first render cycle
  // this will be after the qt scenegraph and render thread has been created for the first window (splash screen)
  QObject::disconnect(pSplashScreen, &QQuickWindow::afterRendering, this, &QtKernel::OnFirstRender);

  // TODO: hook up render thread id stuff again
  //renderThreadId = QThread::currentThreadId();

  // dispatch init call to create our render context and resources on the main thread
  DispatchToMainThread(MakeDelegate(this, &QtKernel::DoInit));
}

// ---------------------------------------------------------------------------------------
void QtKernel::OnAppQuit()
{
  LogTrace("QtKernel::OnAppQuit()");
  Destroy();
}

// ---------------------------------------------------------------------------------------
void QtKernel::OnGLMessageLogged(const QOpenGLDebugMessage &debugMessage)
{
  // TODO: improve the formatting/verbosity of this
  LogDebug(2, SharedString::concat("GL Message: ", debugMessage.message().toUtf8().data()));
}

// ---------------------------------------------------------------------------------------
void QtKernel::DoInit(ep::Kernel *)
{
  LogTrace("QtKernel::DoInit()");

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
  QObject::connect(pGLDebugLogger, &QOpenGLDebugLogger::messageLogged, this, &QtKernel::OnGLMessageLogged);

  if (pGLDebugLogger->initialize())
  {
    // enable synchronous if we need to preserve the gl debug logging order
    //pGLDebugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    pGLDebugLogger->startLogging();
  }

  // init the HAL's render system
  kernel::Kernel::InitRender();

  // app specific init
  kernel::Kernel::DoInit(this);
}

// ---------------------------------------------------------------------------------------
void QtKernel::customEvent(QEvent *pEvent)
{
  if (pEvent->type() == KernelEvent::type())
  {
    MainThreadCallback d;
    d.SetMemento(static_cast<KernelEvent*>(pEvent)->m);
    size_t errorDepth = ErrorLevel();
    try
    {
      d(this);
      if (ErrorLevel() > errorDepth)
      {
        LogError("Exception occurred in MainThreadCallback : {0}", GetError()->message);
        PopErrorToLevel(errorDepth);
      }
    }
    catch (std::exception &e)
    {
      LogError("Exception occurred in MainThreadCallback : {0}", e.what());
      PopErrorToLevel(errorDepth);
    }
    catch (...)
    {
      LogError("Exception occurred in MainThreadCallback : C++ Exception");
      PopErrorToLevel(errorDepth);
    }
  }
  else
  {
    LogWarning(2, SharedString::concat("Unknown event received in Kernel: TYPE ", (int)pEvent->type()));
  }
}

} // namespace qt


/** Kernel ***********************************************/

namespace kernel
{

// ---------------------------------------------------------------------------------------
Kernel *Kernel::CreateInstanceInternal(Slice<const KeyValuePair> commandLine)
{
  return new qt::QtKernel(commandLine);
}

// ---------------------------------------------------------------------------------------
ViewRef Kernel::SetFocusView(ViewRef spView)
{
  if (!spView && spFocusView)
    spFocusView->GetImpl<ViewImpl>()->SetLatestFrame(nullptr);

  ViewRef spOld = spFocusView;
  spFocusView = spView;
  return spOld;
}

// ---------------------------------------------------------------------------------------
void Kernel::Terminate()
{
  qt::QtKernel *pKernel = static_cast<qt::QtKernel*>(this);
  pKernel->TopLevelWindow()->close();
}

// ---------------------------------------------------------------------------------------
void Kernel::DispatchToMainThread(MainThreadCallback callback)
{
  qt::QtKernel *pKernel = static_cast<qt::QtKernel*>(this);

  // if we're on the main thread just execute the callback now
  if (pKernel->OnMainThread())
    callback(this);
  // otherwise jam it into the event queue
  else
    pKernel->PostEvent(new qt::KernelEvent(callback.GetMemento()), Qt::NormalEventPriority);
}

// ---------------------------------------------------------------------------------------
void Kernel::DispatchToMainThreadAndWait(MainThreadCallback callback)
{
  LogTrace("Kernel::DispatchToMainThreadAndWait()");

  qt::QtKernel *pKernel = static_cast<qt::QtKernel*>(this);

  // TODO: handle this gracefully? can we detect if the main thread is blocked??
  EPASSERT(!pKernel->OnRenderThread(), "DispatchToMainThreadAndWait() should not be called on the Render Thread");

  // if we're on the main thread just execute the callback now
  if (pKernel->OnMainThread())
  {
    callback(this);
  }
  // otherwise jam it into the event queue
  else
  {
    QSemaphore sem;
    pKernel->PostEvent(new qt::KernelSyncEvent(callback.GetMemento(), &sem), Qt::NormalEventPriority);
    sem.acquire();
  }
}

} // namespace kernel

#else
EPEMPTYFILE
#endif
