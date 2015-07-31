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
  QtKernel::QtKernel(udInitParams commandLine);
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

// Qt type conversion to/from variants

inline udVariant udToVariant(const QString &string)
{
  QByteArray byteArray = string.toUtf8();

  // Need to do a deep copy and give ownership to the variant
  size_t length = byteArray.size()+1;
  char *pString = udAllocType(char, length, udAF_None);
  memcpy(pString, byteArray.data(), length);

  udString udStr(pString, length);
  return udVariant(udStr, true);
}

inline void udFromVariant(const udVariant &variant, QString *pString)
{
  // NOTE: assumes pString is already empty
  udString s = variant.asString();
  if (!s.empty())
    pString->append(QString::fromUtf8(s.ptr, s.length));
}

#endif  // UDQTKERNEL_INTERNAL_H
