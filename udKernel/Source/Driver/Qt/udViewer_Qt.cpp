#include "udDriver.h"

#if UDINPUT_DRIVER == UDDRIVER_QT

#include <QGuiApplication>
#include <QQuickWindow>
#include <QThread>
#include <QSemaphore>

#include "udKernel.h"


// custom kernel event
class udQtKernelEvent : public QEvent
{
public:
  udQtKernelEvent(const DelegateMemento &mem) : QEvent(type()), m(mem) {}
  virtual ~udQtKernelEvent() {}

  static QEvent::Type type()
  {
    if (eventType == QEvent::None)
      eventType = (QEvent::Type)QEvent::registerEventType();
    return eventType;
  }

  DelegateMemento m;

private:
  static QEvent::Type eventType;
};
QEvent::Type udQtKernelEvent::eventType = QEvent::None;

// custom kernel event with semaphore
class udQtKernelSyncEvent : public udQtKernelEvent
{
public:
  udQtKernelSyncEvent(const DelegateMemento &mem, QSemaphore *pS) : udQtKernelEvent(mem), pSem(pS) {}
  virtual ~udQtKernelSyncEvent() { if(pSem) pSem->release(); }

  QSemaphore *pSem;
};


class udQtKernel : public udKernel, public QObject
{
public:
  udQtKernel::udQtKernel(udInitParams commandLine);
  virtual ~udQtKernel() {}

  udResult Init();
  udResult Shutdown();
  udResult RegisterWindows();
  udResult RunMainLoop();

  bool OnMainThread() { return (mainThreadId == QThread::currentThreadId()); }

  void PostEvent(QEvent *pEvent, int priority = Qt::NormalEventPriority);

private slots:
  void InitRender();
  void CleanupRender();
  void OnSceneGraphInitialized();
  void OnOpenglContextCreated(QOpenGLContext * context);
  void Destroy();

private:
  void RegisterWindow(QQuickWindow *pWin);

  void customEvent(QEvent *pEvent);

  // Members
  int argc;
  udRCSlice<char *> argv;

  QGuiApplication *pApplication;
  QQuickWindow *pMainWindow;
  Qt::HANDLE mainThreadId;
};


// ---------------------------------------------------------------------------------------
udQtKernel::udQtKernel(udInitParams commandLine)
  : QObject(0)
  , pApplication(nullptr)
  , pMainWindow(nullptr)
  , mainThreadId(QThread::currentThreadId())
{
  udDebugPrintf("udQtKernel::udQtKernel()\n");

  // convert udInitParams back into a string list for Qt
  // NOTE: this assumes that the char* list referred to by commandLine will remain valid for the entire lifetime of the udKernel
  // NOTE: the state of our argv may be changed by Qt as it removes args that it recognises
  udFixedSlice<char *> args;
  args.reserve(commandLine.length);
  argc = static_cast<int>(commandLine.length);
  for (int i = 0; i < argc; i++)
    args.pushBack(const_cast<char*>(commandLine.ptr[i].value.asString().ptr));

  argv = args.slice(0, argc);
}

// ---------------------------------------------------------------------------------------
udResult udQtKernel::Init()
{
  udDebugPrintf("udQtKernel::Init()\n");

  // TODO: remove these checks once we are confident in udKernel and the Qt driver
  UDASSERT(argc >= 1, "argc must contain at least 1");
  UDASSERT(argv.length == argc, "argv length should match argc");

  // create our qapplication
  pApplication = new QGuiApplication(argc, argv.ptr);

  // make sure we cleanup the kernel when we're about to quit
  QObject::connect(pApplication, &QCoreApplication::aboutToQuit, this, &udQtKernel::Destroy);

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult udQtKernel::Shutdown()
{
  udDebugPrintf("udQtKernel::Shutdown()\n");
  delete pMainWindow;
  delete this;
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult udQtKernel::RegisterWindows()
{
  udDebugPrintf("udQtKernel::RegisterWindows()\n");

  // TODO: remove these checks once we are confident in udKernel and the Qt driver
  UDASSERT(pApplication != nullptr, "QApplication doesn't exist");

  udResult result = udR_Success;
  QWindowList appWindows = pApplication->allWindows();
  QQuickWindow *pWindow = nullptr;

  UD_ERROR_IF(appWindows.size() == 0, udR_Failure_);

  // TODO: allow for multiple windows?
  pWindow = qobject_cast<QQuickWindow *>(pApplication->allWindows().at(0));

  // TODO: support widget based qt apps?
  UD_ERROR_NULL(pWindow, udR_Failure_);

  RegisterWindow(pWindow);

epilogue:
  if (result != udR_Success)
    udDebugPrintf("App must create a QQuickWindow before calling RunMainLoop()\n");

  return result;
}

// ---------------------------------------------------------------------------------------
udResult udQtKernel::RunMainLoop()
{
  udDebugPrintf("udQtKernel::RunMainLoop()\n");

  // TODO: remove these checks once we are confident in udKernel and the Qt driver
  UDASSERT(pApplication != nullptr, "QApplication doesn't exist");

  // run the Qt event loop - this may never return
  return (pApplication->exec() == 0) ? udR_Success : udR_Failure_;
}

// ---------------------------------------------------------------------------------------
void udQtKernel::PostEvent(QEvent *pEvent, int priority)
{
  // TODO: remove these checks once we are confident in udKernel and the Qt driver
  UDASSERT(pApplication != nullptr, "QApplication doesn't exist");

  pApplication->postEvent(this, pEvent, priority);
}

// ---------------------------------------------------------------------------------------
void udQtKernel::RegisterWindow(QQuickWindow *pWin)
{
  udDebugPrintf("udQtKernel::RegisterWindow()\n");

  // TODO: allow for multiple windows or destroy old window?
  UDASSERT(pMainWindow == nullptr, "We're trying to register a window and one already exists");
  UDASSERT(pWin != nullptr, "Trying to register a null window");
  pMainWindow = pWin;

  // hook into render thread events
  // TODO: we should probably break this out to allow for context destruction/construction
  QObject::connect(pMainWindow, &QQuickWindow::beforeRendering, this, &udQtKernel::InitRender, Qt::DirectConnection);
  //QObject::connect(pMainWindow, &QQuickWindow::sceneGraphInitialized, this, &udQtKernel::OnSceneGraphInitialized, Qt::DirectConnection);
  //QObject::connect(pMainWindow, &QQuickWindow::openglContextCreated, this, &udQtKernel::OnOpenglContextCreated, Qt::DirectConnection);
  QObject::connect(pMainWindow, &QQuickWindow::sceneGraphInvalidated, this, &udQtKernel::CleanupRender, Qt::DirectConnection);
}

// ---------------------------------------------------------------------------------------
void udQtKernel::customEvent(QEvent *pEvent)
{
  if (pEvent->type() == udQtKernelEvent::type())
  {
    MainThreadCallback d;
    d.SetMemento(static_cast<udQtKernelEvent*>(pEvent)->m);
    d(this);
  }
  else
  {
    udDebugPrintf("Unknown event received in Kernel\n");
  }
}

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void udQtKernel::InitRender()
{
  udDebugPrintf("udQtKernel::InitRender()\n");

  // TODO: remove these checks once we are confident in udKernel and the Qt driver
  UDASSERT(pApplication != nullptr, "QApplication doesn't exist");
  UDASSERT(pMainWindow != nullptr, "Main Window doesn't exist");

  // we only want this called on the first render cycle
  QObject::disconnect(pMainWindow, &QQuickWindow::beforeRendering, this, &udQtKernel::InitRender);

  if (udKernel::InitRender() != udR_Success)
  {
    // TODO: gracefully handle error with InitRender ?
    udDebugPrintf("Error initialising renderer\n");
    pApplication->quit();
  }
}

// ---------------------------------------------------------------------------------------
// RENDER THREAD
void udQtKernel::CleanupRender()
{
  // TODO
  udDebugPrintf("CleanupRender\n");
  if (udKernel::DeinitRender() != udR_Success)
  {
    // TODO: gracefully handle error with DeinitRender ?
    udDebugPrintf("Error cleaning up renderer\n");
  }
}

// ---------------------------------------------------------------------------------------
void udQtKernel::Destroy()
{
  udDebugPrintf("udQtKernel::Destroy()\n");
  udKernel::Destroy();
}


// ---------------------------------------------------------------------------------------
udKernel *udKernel::CreateInstanceInternal(udInitParams commandLine)
{
  udDebugPrintf("udKernel::CreateInstanceInternal()\n");
  return new udQtKernel(commandLine);
}

// ---------------------------------------------------------------------------------------
udResult udKernel::InitInstanceInternal()
{
  udDebugPrintf("udKernel::InitInstanceInternal()\n");
  return static_cast<udQtKernel*>(this)->Init();
}

// ---------------------------------------------------------------------------------------
udResult udKernel::InitRenderInternal()
{
  UDASSERT(false, "udKernel::InitRenderInternal() is expected to be unused by the Qt driver");
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult udKernel::DestroyInstanceInternal()
{
  udDebugPrintf("udKernel::DestroyInstanceInternal()\n");
  return static_cast<udQtKernel*>(this)->Shutdown();
}

// ---------------------------------------------------------------------------------------
udViewRef udKernel::SetFocusView(udViewRef spView)
{
  udDebugPrintf("udKernel::SetFocusView()\n");
  udViewRef spOld = spFocusView;
  spFocusView = spView;
  return spOld;
}

// ---------------------------------------------------------------------------------------
udResult udKernel::RunMainLoop()
{
  udDebugPrintf("udKernel::RunMainLoop()\n");

  udResult result;
  udQtKernel *pKernel = static_cast<udQtKernel*>(this);

  UD_ERROR_CHECK(pKernel->RegisterWindows());
  UD_ERROR_CHECK(pKernel->RunMainLoop());

epilogue:
  if (result != udR_Success)
    udDebugPrintf("Error encountered in udKernel::RunMainLoop()\n");

  return result;
}

// ---------------------------------------------------------------------------------------
udResult udKernel::Terminate()
{
  udDebugPrintf("udKernel::Terminate()\n");

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void udKernel::DispatchToMainThread(MainThreadCallback callback)
{
  udDebugPrintf("udKernel::DispatchToMainThread()\n");

  udQtKernel *pKernel = static_cast<udQtKernel*>(this);

  // if we're on the main thread just execute the callback now
  if (pKernel->OnMainThread())
    callback(this);
  // otherwise jam it into the event queue
  else
    pKernel->PostEvent(new udQtKernelEvent(callback.GetMemento()), Qt::NormalEventPriority);
}

// ---------------------------------------------------------------------------------------
void udKernel::DispatchToMainThreadAndWait(MainThreadCallback callback)
{
  udDebugPrintf("udKernel::DispatchToMainThreadAndWait()\n");

  // TODO: check that we're not on the render thread - that's a deadlock waiting to happen

  udQtKernel *pKernel = static_cast<udQtKernel*>(this);

  // if we're on the main thread just execute the callback now
  if (pKernel->OnMainThread())
  {
    callback(this);
  }
  // otherwise jam it into the event queue
  else
  {
    QSemaphore sem;
    pKernel->PostEvent(new udQtKernelSyncEvent(callback.GetMemento(), &sem), Qt::NormalEventPriority);
    sem.acquire();
  }
}

#endif
