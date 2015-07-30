#pragma once
#ifndef UDQTKERNEL_INTERNAL_H
#define UDQTKERNEL_INTERNAL_H

#include <QGuiApplication>
#include <QQmlEngine>
#include <QThread>
#include <QEvent>

#include "kernel.h"
#include "ui/qtuicomponent.h"

namespace qt
{

class Window;

class QtKernel : public ud::Kernel, public QObject
{
public:
  QtKernel::QtKernel(InitParams commandLine);
  virtual ~QtKernel() {}

  udResult Init();
  udResult Shutdown();
  udResult FormatMainWindow(UIComponentRef spUIComponent);
  udResult RunMainLoop();

  bool OnMainThread() { return (mainThreadId == QThread::currentThreadId()); }
  bool OnRenderThread() { return (renderThreadId == QThread::currentThreadId()); }

  void PostEvent(QEvent *pEvent, int priority = Qt::NormalEventPriority);

  QQmlEngine *QmlEngine() { return pQmlEngine; }

private slots:
  void InitRender();
  void CleanupRender();
  void Destroy();

private:
  void customEvent(QEvent *pEvent);

  // Members
  int argc;
  udRCSlice<char *> argv;

  QGuiApplication *pApplication;
  QQmlEngine *pQmlEngine;

  Window *pMainWindow;

  Qt::HANDLE mainThreadId;
  Qt::HANDLE renderThreadId;
};

} // namespace qt

#endif  // UDQTKERNEL_INTERNAL_H
