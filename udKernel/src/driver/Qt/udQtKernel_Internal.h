#pragma once
#ifndef UDQTKERNEL_INTERNAL_H
#define UDQTKERNEL_INTERNAL_H

#include <QGuiApplication>
#include <QQmlEngine>
#include <QThread>
#include <QEvent>
#include <QMetaType>

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
  size_t length = byteArray.size()+1;
  char *pString = udAllocType(char, length, udAF_None);
  memcpy(pString, byteArray.data(), length);

  return udString(pString, length-1);
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
  switch (static_cast<QMetaType::Type>(var.type()))
  {
    // udVariant::Type::Bool
    case QMetaType::Bool:
      return udVariant(var.toBool());

    // udVariant::Type::Float
    case QMetaType::Float:
    case QMetaType::Double:
      return udVariant(var.toDouble());

    // udVariant::Type::Int:
    case QMetaType::Int:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::QChar:
    case QMetaType::UInt:
      return udVariant(var.toLongLong());

    // udVariant::Type::String
    // TODO: optimise - reduce unnecessary copies
    case QMetaType::QByteArray:
    case QMetaType::QString:
      return udVariant(AllocUDStringFromQString(var.toString()), true);

    // TODO: serialize other types?

    // udVariant::Type::Array
    // udVariant::Type::AssocArray
    // udVariant::Type::Null
    default:
      udDebugPrintf("udToVariant: Unsupported type '%s'\n", var.typeName());
      return udVariant();
  };
}

inline void udFromVariant(const udVariant &variant, QVariant *pVariant)
{
  UDASSERT(pVariant->isNull(), "pVariant is not null");

  switch (variant.type())
  {
    //case udVariant::Type::Null:
    case udVariant::Type::Bool:
      pVariant->setValue(variant.asBool());
      break;

    case udVariant::Type::Int:
      pVariant->setValue(variant.asInt());
      break;

    case udVariant::Type::Float:
      pVariant->setValue(variant.asFloat());
      break;

    //case udVariant::Type::Component:
    //case udVariant::Type::Delegate:
    // TODO: optimise?
    case udVariant::Type::String:
      pVariant->setValue(variant.as<QString>());
      break;

    //case udVariant::Type::Array:
    //case udVariant::Type::AssocArray:
    default:
      udDebugPrintf("udToVariant: Unsupported type '%d'\n", variant.type());
  };
}

#endif  // UDQTKERNEL_INTERNAL_H
