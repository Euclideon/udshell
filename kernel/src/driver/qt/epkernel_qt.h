#pragma once
#ifndef epkernel_qt_H
#define epkernel_qt_H

#include <QGuiApplication>
#include <QQmlEngine>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QThread>
#include <QEvent>

#include "kernel.h"

class QQuickWindow;

namespace qt
{

class QtApplication : public QGuiApplication
{
  Q_OBJECT

public:
  QtApplication(kernel::Kernel *pKern, int &argc, char ** argv) : QGuiApplication(argc, argv), pKernel(pKern) {}

  static void SetKernel(kernel::Kernel *pKernel) {
    EPASSERT(qobject_cast<QtApplication*>(QtApplication::instance()), "No valid QtApplication instance");
    static_cast<QtApplication*>(QtApplication::instance())->pKernel = pKernel;
  }
  static kernel::Kernel *Kernel() {
    EPASSERT(qobject_cast<QtApplication*>(QtApplication::instance()), "No valid QtApplication instance");
    return static_cast<QtApplication*>(QtApplication::instance())->pKernel;
  }

protected:
  kernel::Kernel *pKernel = nullptr;
};


class QtKernel : public QObject, public kernel::Kernel
{
  Q_OBJECT

public:
  QtKernel(InitParams commandLine);
  virtual ~QtKernel();

  epResult InitInternal() override;
  epResult RunMainLoop() override;

  bool OnMainThread() { return (mainThreadId == QThread::currentThreadId()); }
  bool OnRenderThread() { return (renderThreadId == QThread::currentThreadId()); }

  void PostEvent(QEvent *pEvent, int priority = Qt::NormalEventPriority);

  QQmlEngine *QmlEngine() { return pQmlEngine; }

  epResult RegisterWindow(QQuickWindow *pWindow);

private slots:
  void OnGLContextCreated(QOpenGLContext *pContext);
  void OnFirstRender();
  void OnAppQuit();

private:
  void DoInit(ep::Kernel *);
  void customEvent(QEvent *pEvent);

  // Members
  int argc;
  SharedArray<char *> argv;

  QtApplication *pApplication;
  QQmlEngine *pQmlEngine;
  QOpenGLContext *pMainThreadContext;

  QSurfaceFormat mainSurfaceFormat;
  QQuickWindow *pTopLevelWindow;

  Qt::HANDLE mainThreadId;
  Qt::HANDLE renderThreadId;
};

} // namespace qt

#endif  // epkernel_qt_H
