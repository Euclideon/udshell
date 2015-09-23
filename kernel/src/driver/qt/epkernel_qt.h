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
  QtApplication(ud::Kernel *pKern, int &argc, char ** argv) : QGuiApplication(argc, argv), pKernel(pKern) {}

  static void SetKernel(ud::Kernel *pKernel) {
    UDASSERT(qobject_cast<QtApplication*>(QtApplication::instance()), "No valid QtApplication instance");
    static_cast<QtApplication*>(QtApplication::instance())->pKernel = pKernel;
  }
  static ud::Kernel *Kernel() {
    UDASSERT(qobject_cast<QtApplication*>(QtApplication::instance()), "No valid QtApplication instance");
    return static_cast<QtApplication*>(QtApplication::instance())->pKernel;
  }

protected:
  ud::Kernel *pKernel = nullptr;
};


class QtKernel : public QObject, public ud::Kernel
{
  Q_OBJECT

public:
  QtKernel(udInitParams commandLine);
  virtual ~QtKernel();

  udResult InitInternal() override;
  udResult RunMainLoop() override;

  bool OnMainThread() { return (mainThreadId == QThread::currentThreadId()); }
  bool OnRenderThread() { return (renderThreadId == QThread::currentThreadId()); }

  void PostEvent(QEvent *pEvent, int priority = Qt::NormalEventPriority);

  QQmlEngine *QmlEngine() { return pQmlEngine; }

  udResult RegisterWindow(QQuickWindow *pWindow);

private slots:
  void OnGLContextCreated(QOpenGLContext *pContext);
  void OnFirstRender();
  void OnAppQuit();

private:
  void DoInit(ud::Kernel *);
  void customEvent(QEvent *pEvent);

  // Members
  int argc;
  udSharedSlice<char *> argv;

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
