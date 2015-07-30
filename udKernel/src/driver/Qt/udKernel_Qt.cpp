#include "hal/driver.h"

#if UDWINDOW_DRIVER == UDDRIVER_QT

#include <QSemaphore>

#include "udQtKernel_Internal.h"

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
QtKernel::QtKernel(InitParams commandLine)
  : QObject(0)
  , pApplication(nullptr)
  , pMainWindow(nullptr)
  , pQmlEngine(nullptr)
  , mainThreadId(QThread::currentThreadId())
  , renderThreadId(nullptr)
{
  udDebugPrintf("QtKernel::udQtKernel()\n");

  // convert InitParams back into a string list for Qt
  // NOTE: this assumes that the char* list referred to by commandLine will remain valid for the entire lifetime of the Kernel
  // NOTE: the state of our argv may be changed by Qt as it removes args that it recognises
  udFixedSlice<char *> args;
  args.reserve(commandLine.length);
  argc = static_cast<int>(commandLine.length);
  for (int i = 0; i < argc; i++)
    args.pushBack(const_cast<char*>(commandLine.ptr[i].value.asString().ptr));

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

  // TODO: expose kernel innards to the qml context?

  pMainWindow = new QQuickWindow();

  QObject::connect(pMainWindow, &QQuickWindow::beforeRendering, this, &QtKernel::InitRender, Qt::DirectConnection);
  QObject::connect(pMainWindow, &QQuickWindow::sceneGraphInvalidated, this, &QtKernel::CleanupRender, Qt::DirectConnection);

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult QtKernel::Shutdown()
{
  udDebugPrintf("QtKernel::Shutdown()\n");
  delete pMainWindow;
  delete pQmlEngine;
  delete this;
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult QtKernel::FormatMainWindow(UIComponentRef spUIComponent)
{
  udDebugPrintf("QtKernel::FormatMainWindow()\n");

  // NOTE: we are reparenting the "visual parent" of the ui component, this means ui component is still
  // responsible for cleaning up its qobject
  // TODO: should we take complete ownership of the ui component??
  spUIComponent->QuickItem()->setParentItem(pMainWindow->contentItem());

  // TODO: store this info as properties?
  pMainWindow->resize(800, 600);
  pMainWindow->show();
  pMainWindow->raise();

  // TODO: wire this up to a resize event
  spUIComponent->QuickItem()->setWidth(800);
  spUIComponent->QuickItem()->setHeight(600);

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

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void QtKernel::InitRender()
{
  udDebugPrintf("QtKernel::InitRender()\n");

  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  UDASSERT(pApplication != nullptr, "QApplication doesn't exist");
  UDASSERT(pMainWindow != nullptr, "Main Window doesn't exist");

  // we only want this called on the first render cycle
  QObject::disconnect(pMainWindow, &QQuickWindow::beforeRendering, this, &QtKernel::InitRender);

  // TODO: need a better place to set this since this is called *after* our scenegraph has been created
  // ALSO does this thread ever get recreated? does this id remain valid thru the program?
  renderThreadId = QThread::currentThreadId();

  if (ud::Kernel::InitRender() != udR_Success)
  {
    // TODO: gracefully handle error with InitRender ?
    udDebugPrintf("Error initialising renderer\n");
    pApplication->quit();
  }
}

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void QtKernel::CleanupRender()
{
  udDebugPrintf("QtKernel::CleanupRender\n");
  if (ud::Kernel::DeinitRender() != udR_Success)
  {
    // TODO: gracefully handle error with DeinitRender ?
    udDebugPrintf("Error cleaning up renderer\n");
  }
}

// ---------------------------------------------------------------------------------------
void QtKernel::Destroy()
{
  udDebugPrintf("QtKernel::Destroy()\n");
  ud::Kernel::Destroy();
}

} // namespace qt


/** Kernel ***********************************************/

namespace ud
{

// ---------------------------------------------------------------------------------------
Kernel *Kernel::CreateInstanceInternal(InitParams commandLine)
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
udResult Kernel::InitRenderInternal()
{
  UDASSERT(false, "Kernel::InitRenderInternal() is expected to be unused by the Qt driver");
  return udR_Success;
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
udResult Kernel::FormatMainWindow(UIComponentRef spUIComponent)
{
  udDebugPrintf("Kernel::FormatMainWindow()\n");
  return static_cast<qt::QtKernel*>(this)->FormatMainWindow(spUIComponent);
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
  udDebugPrintf("Kernel::DispatchToMainThread()\n");

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
