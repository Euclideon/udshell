#include "hal/driver.h"

#if UDWINDOW_DRIVER == UDDRIVER_QT

#include <QSemaphore>
#include <QQuickWindow>

#include "udQtKernel_Internal.h"
#include "ui/window.h"
#include "ui/renderview.h"

// Init the kernel's qrc file resources - this has to happen from the global namespace
inline void InitResources() { Q_INIT_RESOURCE(kernel); }

namespace qt
{

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
QtKernel::QtKernel(udInitParams commandLine)
  : QObject(0)
  , pApplication(nullptr)
  , pQmlEngine(nullptr)
  , pMainThreadContext(nullptr)
  , mainSurfaceFormat(QSurfaceFormat::defaultFormat())
  , pTopLevelWindow(nullptr)
  , mainThreadId(QThread::currentThreadId())
  , renderThreadId(nullptr)
{
  udDebugPrintf("QtKernel::udQtKernel()\n");

  // convert udInitParams back into a string list for Qt
  // NOTE: this assumes that the char* list referred to by commandLine will remain valid for the entire lifetime of the Kernel
  // NOTE: the state of our argv may be changed by Qt as it removes args that it recognises
  udFixedSlice<char *, 1> args;
  args.reserve(commandLine.params.length);
  argc = static_cast<int>(commandLine.params.length);
  for (int i = 0; i < argc; i++)
    args.pushBack(const_cast<char*>(commandLine[i].value.asString().ptr));

  argv = args.slice(0, argc);
}

// ---------------------------------------------------------------------------------------
udResult QtKernel::Init()
{
  udDebugPrintf("QtKernel::Init()\n");

  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  UDASSERT(argc >= 1, "argc must contain at least 1");
  UDASSERT(argv.length == argc, "argv length should match argc");

  // create our qapplication
  pApplication = new QGuiApplication(argc, argv.ptr);

  // make sure we cleanup the kernel when we're about to quit
  QObject::connect(pApplication, &QCoreApplication::aboutToQuit, this, &QtKernel::Destroy);

  pQmlEngine = new QQmlEngine(this);

  // register our internal qml types
  qmlRegisterType<RenderView>("udKernel", 0, 1, "RenderView");

  // TODO: expose kernel innards to the qml context?

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
  pTopLevelWindow = qobject_cast<QQuickWindow*>(object);
  if (!pTopLevelWindow)
  {
    // TODO: better error information/handling
    udDebugPrintf("Error creating Splash Screen\n");
    foreach(const QQmlError &error, component.errors())
      udDebugPrintf("QML ERROR: %s\n", error.toString().toLatin1().data());
    return udR_Failure_;
  }

  // defer the heavier init stuff and app specific init to after Qt hits the event loop
  // we'll hook into the splash screen to do this
  QObject::connect(pTopLevelWindow, &QQuickWindow::afterRendering, this, &QtKernel::OnFirstRender, Qt::DirectConnection);

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult QtKernel::Shutdown()
{
  udDebugPrintf("QtKernel::Shutdown()\n");

  delete pTopLevelWindow;

  if (ud::Kernel::DeinitRender() != udR_Success)
  {
    // TODO: gracefully handle error with DeinitRender ?
    udDebugPrintf("Error cleaning up renderer\n");
  }

  delete pQmlEngine;
  delete this;
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult QtKernel::SetTopLevelUI(QtWindowRef spWindow)
{
  udDebugPrintf("QtKernel::SetTopLevelUI()\n");

  QQuickWindow *pPrevWindow = pTopLevelWindow;
  pTopLevelWindow = spWindow->QuickWindow();

  if (pPrevWindow)
  {
    // unhook the current top level window
    //QObject::disconnect(pPrevWindow, &QQuickWindow::sceneGraphInitialized, this, &QtKernel::OnSceneGraphInitialized);
    QObject::disconnect(pTopLevelWindow, &QQuickWindow::openglContextCreated, this, &QtKernel::OnGLContextCreated);
    pPrevWindow->hide();
    delete pPrevWindow;
  }

  pMainThreadContext->doneCurrent();

  // hook up the new one
  //QObject::connect(pTopLevelWindow, &QQuickWindow::sceneGraphInitialized, this, &QtKernel::OnSceneGraphInitialized, Qt::DirectConnection);
  QObject::connect(pTopLevelWindow, &QQuickWindow::openglContextCreated, this, &QtKernel::OnGLContextCreated, Qt::DirectConnection);

  pTopLevelWindow->show();
  pTopLevelWindow->raise();

  if (!pMainThreadContext->makeCurrent(pTopLevelWindow))
  {
    // TODO: error
    udDebugPrintf("Error making main gl context current\n");
    return udR_Failure_;
  }

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult QtKernel::RunMainLoop()
{
  udDebugPrintf("QtKernel::RunMainLoop()\n");

  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  UDASSERT(pApplication != nullptr, "QApplication doesn't exist");

  // run the Qt event loop - this may never return
  return (pApplication->exec() == 0) ? udR_Success : udR_Failure_;
}

// ---------------------------------------------------------------------------------------
void QtKernel::PostEvent(QEvent *pEvent, int priority)
{
  udDebugPrintf("QtKernel::PostEvent()\n");
  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  UDASSERT(pApplication != nullptr, "QApplication doesn't exist");

  pApplication->postEvent(this, pEvent, priority);
}

// ---------------------------------------------------------------------------------------
void QtKernel::OnGLContextCreated(QOpenGLContext *pContext)
{
  udDebugPrintf("QtKernel::CreateOpenGLContext()\n");

  UDASSERT(pMainThreadContext != nullptr, "Expected GL context");

  // we need to share our context with Qt and recreate
  pContext->setShareContext(pMainThreadContext);
  bool succeed = pContext->create();

  // TODO: error handle
  UDASSERT(succeed, "Couldn't create shared render context!");
}

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void QtKernel::OnFirstRender()
{
  udDebugPrintf("QtKernel::OnFirstRender()\n");

  // we only want this called on the first render cycle
  QObject::disconnect(pTopLevelWindow, &QQuickWindow::afterRendering, this, &QtKernel::OnFirstRender);

  // TODO: hook up render thread id stuff again
  //renderThreadId = QThread::currentThreadId();

  // dispatch init call
  DispatchToMainThread(MakeDelegate(this, &QtKernel::DoInit));
}

// ---------------------------------------------------------------------------------------
void QtKernel::Destroy()
{
  udDebugPrintf("QtKernel::Destroy()\n");
  ud::Kernel::Destroy();
}

// ---------------------------------------------------------------------------------------
void QtKernel::DoInit(ud::Kernel *)
{
  udDebugPrintf("QtKernel::DoInit()\n");

  UDASSERT(pTopLevelWindow != nullptr, "No active window set");
  UDASSERT(pMainThreadContext == nullptr, "Main Thread context already exists");

  // create main opengl context
  pMainThreadContext = new QOpenGLContext();
  pMainThreadContext->setFormat(mainSurfaceFormat);
  bool succeed = pMainThreadContext->create();
  UDASSERT(succeed, "Couldn't create render context!");

  if (!pMainThreadContext->makeCurrent(pTopLevelWindow))
  {
    // TODO: handle error
    udDebugPrintf("Error making main gl context current\n");
    pApplication->quit();
  }

  // init the HAL's render system
  if (ud::Kernel::InitRender() != udR_Success)
  {
    // TODO: gracefully handle error with InitRender ?
    udDebugPrintf("Error initialising renderer\n");
    pApplication->quit();
  }

  // app specific init
  ud::Kernel::DoInit(this);
}

// ---------------------------------------------------------------------------------------
void QtKernel::customEvent(QEvent *pEvent)
{
  udDebugPrintf("QtKernel::customEvent()\n");
  if (pEvent->type() == KernelEvent::type())
  {
    MainThreadCallback d;
    d.SetMemento(static_cast<KernelEvent*>(pEvent)->m);
    d(this);
  }
  else
  {
    udDebugPrintf("Unknown event received in Kernel\n");
  }
}

} // namespace qt


/** Kernel ***********************************************/

namespace ud
{

// ---------------------------------------------------------------------------------------
Kernel *Kernel::CreateInstanceInternal(udInitParams commandLine)
{
  udDebugPrintf("Kernel::CreateInstanceInternal()\n");
  return new qt::QtKernel(commandLine);
}

// ---------------------------------------------------------------------------------------
udResult Kernel::InitInstanceInternal()
{
  udDebugPrintf("Kernel::InitInstanceInternal()\n");
  return static_cast<qt::QtKernel*>(this)->Init();
}

// ---------------------------------------------------------------------------------------
udResult Kernel::DestroyInstanceInternal()
{
  udDebugPrintf("Kernel::DestroyInstanceInternal()\n");
  return static_cast<qt::QtKernel*>(this)->Shutdown();
}

// ---------------------------------------------------------------------------------------
ViewRef Kernel::SetFocusView(ViewRef spView)
{
  udDebugPrintf("Kernel::SetFocusView()\n");
  ViewRef spOld = spFocusView;
  spFocusView = spView;
  return spOld;
}

// ---------------------------------------------------------------------------------------
udResult Kernel::SetTopLevelUI(WindowRef spWindow)
{
  udDebugPrintf("Kernel::SetTopLevelUI()\n");
  return static_cast<qt::QtKernel*>(this)->SetTopLevelUI(spWindow);
}

// ---------------------------------------------------------------------------------------
udResult Kernel::RunMainLoop()
{
  udDebugPrintf("Kernel::RunMainLoop()\n");
  return static_cast<qt::QtKernel*>(this)->RunMainLoop();
}

// ---------------------------------------------------------------------------------------
udResult Kernel::Terminate()
{
  udDebugPrintf("Kernel::Terminate()\n");

  return udR_Success;
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
  udDebugPrintf("Kernel::DispatchToMainThreadAndWait()\n");

  qt::QtKernel *pKernel = static_cast<qt::QtKernel*>(this);

  // TODO: handle this gracefully? can we detect if the main thread is blocked??
  UDASSERT(!pKernel->OnRenderThread(), "DispatchToMainThreadAndWait() should not be called on the Render Thread");

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

} // namespace ud

#endif
