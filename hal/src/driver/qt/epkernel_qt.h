#pragma once
#ifndef epkernel_qt_H
#define epkernel_qt_H

#include "ep/cpp/platform.h"

#include "driver/qt/epqt.h"

#include <QApplication>
#include <QQmlEngine>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QOpenGLDebugLogger>
#include <QSurfaceFormat>
#include <QOffscreenSurface>
#include <QThread>
#include <QEvent>
#include <QPointer>

#include "ep/cpp/kernel.h"

#include "driver/qt/components/qmlpluginloader_qt.h"
#include "driver/qt/ui/focusmanager_qt.h"

class QQuickWindow;

namespace qt
{

class QtKernel;

// Qt's QCoreApplication based classes are treated by Qt as a singleton
// We specialise our own version to store our Kernel pointer and provide global access around the Qt set of classes
class QtApplication : public QApplication
{
  Q_OBJECT

public:
  QtApplication(QtKernel *pKern, int &argc, char ** argv) : QApplication(argc, argv), pKernel(pKern) {}

  static QtKernel *kernel() {
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

  void postEvent(QEvent *pEvent, int priority = Qt::NormalEventPriority);

public slots:
  void onGLContextCreated(QOpenGLContext *pContext);
  void onAppQuit();
  void onFirstRender();
  void onGLMessageLogged(const QOpenGLDebugMessage &debugMessage);

private:
  void customEvent(QEvent *pEvent) override;

  QtKernel *pQtKernel;
};


class QtKernel : public ep::Kernel
{
  EP_DECLARE_COMPONENT(ep, QtKernel, ep::Kernel, EPKERNEL_PLUGINVERSION, "Qt Kernel instance", 0)

public:
  QtKernel(ep::Variant::VarMap commandLine);
  virtual ~QtKernel();

  void runMainLoop() override final;
  void quit() override final;

  ep::ViewRef setFocusView(ep::ViewRef spView) override final;
  void dispatchToMainThread(ep::MainThreadCallback callback) override final;
  void dispatchToMainThreadAndWait(ep::MainThreadCallback callback) override final;

  bool onMainThread() { return (mainThreadId == QThread::currentThreadId()); }
  bool onRenderThread() { return (renderThreadId == QThread::currentThreadId()); }

  QQmlEngine *qmlEngine() const { return pQmlEngine; }

  ep::Result registerWindow(QQuickWindow *pWindow);
  void unregisterWindow(QQuickWindow *pWindow);
  QPointer<QQuickWindow> topLevelWindow() { return pTopLevelWindow; }

  QtFocusManager *getFocusManager() { return pFocusManager; }

  void registerQmlComponent(ep::String file);
  void registerQmlComponents(ep::String folderPath);
  ep::ComponentRef createQmlComponent(ep::String file, ep::Variant::VarMap initParams);

private:
  friend class QtKernelMediator;
  friend class QmlPluginLoader;

  static ep::ComponentDescInl *makeKernelDescriptor();

  void finishInit();
  void shutdown();

  void onFatal(ep::String msg) override final;

  void registerQml(ep::String file, ep::Variant::VarMap desc);

  ep::Array<const ep::MethodInfo> getMethods() const
  {
    return{
      EP_MAKE_METHOD(registerQmlComponent, "Register a new QML Component type"),
      EP_MAKE_METHOD(registerQmlComponents, "Scans the path for valid QML components and registers them"),
      EP_MAKE_METHOD(createQmlComponent, "Creates a new QML Component from file")
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
  QOffscreenSurface *pOffscreenSurface = nullptr;

  QtFocusManager *pFocusManager = nullptr;

  bool singleThreadMode;
  Qt::HANDLE mainThreadId = QThread::currentThreadId();
  Qt::HANDLE renderThreadId = nullptr;

  QmlPluginLoaderRef spQmlPluginLoader = nullptr;
};

} // namespace qt

#endif  // epkernel_qt_H
