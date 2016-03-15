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

class QtApplication : public QGuiApplication
{
  Q_OBJECT

public:
  QtApplication(QtKernel *pKern, int &argc, char ** argv) : QGuiApplication(argc, argv), pKernel(pKern) {}

  static void SetKernel(QtKernel *pKernel) {
    EPASSERT(qobject_cast<QtApplication*>(QtApplication::instance()), "No valid QtApplication instance");
    static_cast<QtApplication*>(QtApplication::instance())->pKernel = pKernel;
  }
  static QtKernel *Kernel() {
    EPASSERT(qobject_cast<QtApplication*>(QtApplication::instance()), "No valid QtApplication instance");
    return static_cast<QtApplication*>(QtApplication::instance())->pKernel;
  }

protected:
  QtKernel *pKernel = nullptr;
};


class QtKernel : public QObject, public Kernel
{
  Q_OBJECT

  EP_DECLARE_COMPONENT(QtKernel, Kernel, EPKERNEL_PLUGINVERSION, "Qt Kernel instance", 0)

public:
  QtKernel(Variant::VarMap commandLine);
  virtual ~QtKernel();

  void RunMainLoop() override final;
  void Quit() override final;

  ViewRef SetFocusView(ViewRef spView) override final;
  void DispatchToMainThread(MainThreadCallback callback) override final;
  void DispatchToMainThreadAndWait(MainThreadCallback callback) override final;

  bool OnMainThread() { return (mainThreadId == QThread::currentThreadId()); }
  bool OnRenderThread() { return (renderThreadId == QThread::currentThreadId()); }

  void PostEvent(QEvent *pEvent, int priority = Qt::NormalEventPriority);

  QQmlEngine *QmlEngine() const { return pQmlEngine; }

  epResult RegisterWindow(QQuickWindow *pWindow);
  void UnregisterWindow(QQuickWindow *pWindow);
  QPointer<QQuickWindow> TopLevelWindow() { return pTopLevelWindow; }

  QtFocusManager *GetFocusManager() { return pFocusManager; }

  void RegisterQmlComponent(String superTypeId, String typeId, String file);
  ep::ComponentRef CreateQmlComponent(String superTypeId, String file);

private slots:
  void OnGLContextCreated(QOpenGLContext *pContext);
  void OnFirstRender();
  void OnAppQuit();
  void OnGLMessageLogged(const QOpenGLDebugMessage &debugMessage);

  void FinishInit();

private:
  static ComponentDescInl *MakeKernelDescriptor();

  void customEvent(QEvent *pEvent) override;

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(RegisterQmlComponent, "Register a new QML Component type"),
      EP_MAKE_METHOD(CreateQmlComponent, "Creates a new QML Component from file")
    };
  }

  // Members
  int argc;
  Array<SharedString> cmdArgs;
  Array<const char *> cmdArgv;

  QtApplication *pApplication;
  QQmlEngine *pQmlEngine;
  QOpenGLContext *pMainThreadContext;
  QOpenGLDebugLogger *pGLDebugLogger;

  QSurfaceFormat mainSurfaceFormat;
  QQuickWindow *pSplashScreen;
  QPointer<QQuickWindow> pTopLevelWindow;

  QtFocusManager *pFocusManager;

  Qt::HANDLE mainThreadId;
  Qt::HANDLE renderThreadId;
};

} // namespace qt

#endif  // epkernel_qt_H
