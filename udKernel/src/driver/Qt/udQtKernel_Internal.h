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
  udResult FormatMainWindow(QtUIComponentRef spUIComponent);
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

// Qt type conversion to/from UD

inline udString AllocUDStringFromQString(const QString &string)
{
  QByteArray byteArray = string.toUtf8();

  // Need to do a deep copy and give ownership to the variant
  size_t length = byteArray.size() + 1;
  char *pString = udAllocType(char, length, udAF_None);
  memcpy(pString, byteArray.data(), length);

  return udString(pString, length);
}

inline udVariant udToVariant(const QString &string)
{
  return udVariant(AllocUDStringFromQString(string), true);
}

inline void udFromVariant(const udVariant &variant, QString *pString)
{
  udString s = variant.asString();
  if (!s.empty())
    *pString = QString::fromUtf8(s.ptr, static_cast<int>(s.length));
}

inline udVariant udToVariant(const QVariant &var)
{
  // TODO: implement qvariant to udvariant
  udDebugPrintf("CONVERT QVariant to udVariant\n");
  udDebugPrintf("VALUE: %s\n", var.toString().toLatin1().data());

  return udVariant();
}

inline void udFromVariant(const udVariant &variant, QVariant *pVariant)
{
  // TODO: implement udvariant to qvariant
  udDebugPrintf("CONVERT udVariant to QVariant\n");
  udDebugPrintf("VALUE: %s\n", variant.asString().toStringz());
}

#endif  // UDQTKERNEL_INTERNAL_H
