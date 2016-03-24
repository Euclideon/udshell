#pragma once
#ifndef epkernel_qt_H
#define epkernel_qt_H

#include "ep/cpp/platform.h"

#include "driver/qt/epqt.h"

#include <QGuiApplication>
#include <QQmlEngine>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QOpenGLDebugLogger>
#include <QSurfaceFormat>
#include <QThread>
#include <QEvent>
#include <QPointer>

#include "ui/focusmanager_qt.h"
#include "ep/cpp/kernel.h"

class QQuickWindow;

namespace qt
{

class QtKernel;

// Qt's QCoreApplication based classes are treated by Qt as a singleton
// We specialise our own version to store our Kernel pointer and provide global access around the Qt set of classes
class QtApplication : public QGuiApplication
{
  Q_OBJECT

public:
  QtApplication(QtKernel *pKern, int &argc, char ** argv) : QGuiApplication(argc, argv), pKernel(pKern) {}

  static QtKernel *Kernel() {
    EPASSERT(qobject_cast<QtApplication*>(QtApplication::instance()), "No valid QtApplication instance");
    return static_cast<QtApplication*>(QtApplication::instance())->pKernel;
  }

protected:
  QtKernel *pKernel = nullptr;
};


class QtKernelMediator : public QObject
{
  Q_OBJECT

public:
  QtKernelMediator(QtKernel *pKernel) : QObject(nullptr), pQtKernel(pKernel) {}
  ~QtKernelMediator() {}

  void PostEvent(QEvent *pEvent, int priority = Qt::NormalEventPriority);

public slots:
  void OnGLContextCreated(QOpenGLContext *pContext);
  void OnAppQuit();
  void OnFirstRender();
  void OnGLMessageLogged(const QOpenGLDebugMessage &debugMessage);

private:
  void customEvent(QEvent *pEvent) override;

  QtKernel *pQtKernel;
};


class QtKernel : public ep::Kernel
{
  EP_DECLARE_COMPONENT(QtKernel, ep::Kernel, EPKERNEL_PLUGINVERSION, "Qt Kernel instance", 0)

public:
  QtKernel(ep::Variant::VarMap commandLine);
  virtual ~QtKernel();

  void RunMainLoop() override final;
  void Quit() override final;

  ep::ViewRef SetFocusView(ep::ViewRef spView) override final;
  void DispatchToMainThread(ep::MainThreadCallback callback) override final;
  void DispatchToMainThreadAndWait(ep::MainThreadCallback callback) override final;

  bool OnMainThread() { return (mainThreadId == QThread::currentThreadId()); }
  bool OnRenderThread() { return (renderThreadId == QThread::currentThreadId()); }

  QQmlEngine *QmlEngine() const { return pQmlEngine; }

  epResult RegisterWindow(QQuickWindow *pWindow);
  void UnregisterWindow(QQuickWindow *pWindow);
  QPointer<QQuickWindow> TopLevelWindow() { return pTopLevelWindow; }

  QtFocusManager *GetFocusManager() { return pFocusManager; }

  void RegisterQmlComponent(ep::String superTypeId, ep::String typeId, ep::String file);
  ep::ComponentRef CreateQmlComponent(ep::String superTypeId, ep::String file, ep::Variant::VarMap initParams);

private:
  friend class QtKernelMediator;

  static ep::ComponentDescInl *MakeKernelDescriptor();

  void FinishInit();
  void Shutdown();

  static ep::Array<const ep::MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(RegisterQmlComponent, "Register a new QML Component type"),
      EP_MAKE_METHOD(CreateQmlComponent, "Creates a new QML Component from file")
    };
  }

  // Members
  int cmdArgc;
  ep::Array<ep::SharedString> cmdArgs;
  ep::Array<const char *> cmdArgv;

  QtKernelMediator *pMediator = nullptr;

  QtApplication *pApplication = nullptr;
  QQmlEngine *pQmlEngine = nullptr;
  QOpenGLContext *pMainThreadContext = nullptr;
  QOpenGLDebugLogger *pGLDebugLogger = nullptr;

  QSurfaceFormat mainSurfaceFormat = QSurfaceFormat::defaultFormat();
  QQuickWindow *pSplashScreen = nullptr;
  QPointer<QQuickWindow> pTopLevelWindow = nullptr;

  QtFocusManager *pFocusManager = nullptr;

  Qt::HANDLE mainThreadId = QThread::currentThreadId();
  Qt::HANDLE renderThreadId = nullptr;
};

} // namespace qt

#endif  // epkernel_qt_H
