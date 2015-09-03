#pragma once
#ifndef SIGNALTODELEGATE_H
#define SIGNALTODELEGATE_H

// suppress warnings from qvariant
#pragma warning(push,3)
#include <QObject>
#include <QMetaMethod>
#pragma warning(pop)

#include "typeconvert.h"
#include "util/foreach.h"

namespace qt
{

// TODO: simplify these macros?
// Ugly macros to simplify creation of new slots
#define QT_SIGNAL_HANDLER_0() \
  void SignalHandler() { \
    udDelegate<void()> del = v.as<udDelegate<void()>>(); \
    del(); \
  }

#define QT_SIGNAL_HANDLER_1(TYPE1) \
  void SignalHandler(TYPE1 arg1) { \
    udDelegate<void(udVariant)> del = v.as<udDelegate<void(udVariant)>>(); \
    del(udVariant(arg1)); \
  }

#define QT_SIGNAL_HANDLER_2(TYPE1, TYPE2) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2) { \
    udDelegate<void(udVariant, udVariant)> del = v.as<udDelegate<void(udVariant,udVariant)>>(); \
    del(udVariant(arg1), udVariant(arg2)); \
  }

#define QT_SIGNAL_HANDLER_3(TYPE1, TYPE2, TYPE3) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2, TYPE3 arg3) { \
    udDelegate<void(udVariant,udVariant,udVariant)> del = v.as<udDelegate<void(udVariant,udVariant,udVariant)>>(); \
    del(udVariant(arg1), udVariant(arg2), udVariant(arg3)); \
  }

#define QT_SIGNAL_HANDLER_4(TYPE1, TYPE2, TYPE3, TYPE4) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2, TYPE3 arg3, TYPE4 arg4) { \
    udDelegate<void(udVariant,udVariant,udVariant,udVariant)> del = v.as<udDelegate<void(udVariant,udVariant,udVariant,udVariant)>>(); \
    del(udVariant(arg1), udVariant(arg2), udVariant(arg3), udVariant(arg4)); \
  }


class QtSignalToDelegate : public QObject
{
  Q_OBJECT

public:
  QtSignalToDelegate(const QObject *pSourceObj, const QMetaMethod &m, const udVariant::Delegate &d) : v(d)
  {
    QMetaMethod sigHandler = lookupSignalHandler(m);
    if (sigHandler.isValid())
      connection = QObject::connect(pSourceObj, m, this, sigHandler);
  }
  ~QtSignalToDelegate()
  {
    if (connection)
      QObject::disconnect(connection);
  }

  operator bool() { return connection; }

private:
  QMetaMethod lookupSignalHandler(const QMetaMethod &m)
  {
    int methodIndex = -1;
    QByteArray methodSig = m.methodSignature();
    int indexOfOpenBracket = methodSig.indexOf('(');
    if (indexOfOpenBracket != -1)
    {
      udMutableString128 signalHandlerName;
      signalHandlerName.concat("SignalHandler", methodSig.right(methodSig.size() - indexOfOpenBracket).data());
      udDebugPrintf("Checking for signal handler: '%s'\n", signalHandlerName.toStringz());
      methodIndex = metaObject()->indexOfMethod(signalHandlerName.toStringz());
    }

    // TODO: remove this before release, maybe allow fallback on most closest match and error log/warn?
    if (methodIndex == -1)
    {
      UDASSERT(false, "Attempted to connect to unsupported signal: '%s' ", m.methodSignature().data());
      return QMetaMethod();
    }

    return metaObject()->method(methodIndex);
  }

private slots:
  QT_SIGNAL_HANDLER_0();
  QT_SIGNAL_HANDLER_1(QVariant);
  QT_SIGNAL_HANDLER_1(QString);
  QT_SIGNAL_HANDLER_1(bool);
  QT_SIGNAL_HANDLER_1(double);
  QT_SIGNAL_HANDLER_1(int);
  QT_SIGNAL_HANDLER_2(QVariant, QVariant);

private:
  QMetaObject::Connection connection;
  udVariant v;
};

} // namespace qt

#endif  // SIGNALTODELEGATE_H
