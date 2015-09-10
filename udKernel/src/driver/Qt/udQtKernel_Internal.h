#pragma once
#ifndef UDQTKERNEL_INTERNAL_H
#define UDQTKERNEL_INTERNAL_H

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

class QtKernel : public ud::Kernel, public QObject
{
public:
  QtKernel(udInitParams commandLine);
  virtual ~QtKernel() {}

  udResult Init();
  udResult Shutdown();
  udResult RunMainLoop();

  bool OnMainThread() { return (mainThreadId == QThread::currentThreadId()); }
  bool OnRenderThread() { return (renderThreadId == QThread::currentThreadId()); }

  void PostEvent(QEvent *pEvent, int priority = Qt::NormalEventPriority);

  QQmlEngine *QmlEngine() { return pQmlEngine; }

  udResult RegisterWindow(QQuickWindow *pWindow);

private slots:
  void OnGLContextCreated(QOpenGLContext *pContext);
  void OnFirstRender();
  void Destroy();

private:
  void DoInit(ud::Kernel *);
  void customEvent(QEvent *pEvent);

  // Members
  int argc;
  udSharedSlice<char *> argv;

  QGuiApplication *pApplication;
  QQmlEngine *pQmlEngine;
  QOpenGLContext *pMainThreadContext;

  QSurfaceFormat mainSurfaceFormat;
  QQuickWindow *pTopLevelWindow;

  Qt::HANDLE mainThreadId;
  Qt::HANDLE renderThreadId;
};

} // namespace qt

#endif  // UDQTKERNEL_INTERNAL_H
