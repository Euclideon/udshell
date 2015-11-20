#pragma once
#ifndef SIGNALTODELEGATE_H
#define SIGNALTODELEGATE_H

// suppress warnings from qt
#if defined(_MSC_VER)
# pragma warning(push,3)
#endif
#include <QObject>
#include <QMetaMethod>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif

#include "typeconvert_qt.h"

namespace qt
{

// TODO: simplify these macros?
// Ugly macros to simplify creation of new slots
#define QT_SIGNAL_HANDLER_0() \
  void SignalHandler() { \
    Delegate<void()> del = v.as<Delegate<void()>>(); \
    del(); \
  }

#define QT_SIGNAL_HANDLER_1(TYPE1) \
  void SignalHandler(TYPE1 arg1) { \
    Delegate<void(Variant)> del = v.as<Delegate<void(Variant)>>(); \
    del(Variant(arg1)); \
  }

#define QT_SIGNAL_HANDLER_2(TYPE1, TYPE2) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2) { \
    Delegate<void(Variant, Variant)> del = v.as<Delegate<void(Variant,Variant)>>(); \
    del(Variant(arg1), Variant(arg2)); \
  }

#define QT_SIGNAL_HANDLER_3(TYPE1, TYPE2, TYPE3) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2, TYPE3 arg3) { \
    Delegate<void(Variant,Variant,Variant)> del = v.as<Delegate<void(Variant,Variant,Variant)>>(); \
    del(Variant(arg1), Variant(arg2), Variant(arg3)); \
  }

#define QT_SIGNAL_HANDLER_4(TYPE1, TYPE2, TYPE3, TYPE4) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2, TYPE3 arg3, TYPE4 arg4) { \
    Delegate<void(Variant,Variant,Variant,Variant)> del = v.as<Delegate<void(Variant,Variant,Variant,Variant)>>(); \
    del(Variant(arg1), Variant(arg2), Variant(arg3), Variant(arg4)); \
  }


class QtSignalToDelegate : public QObject
{
  Q_OBJECT

public:
  QtSignalToDelegate(const QtSignalToDelegate &rh)
    : QObject(), connection(rh.connection), v(rh.v)
  {}
  QtSignalToDelegate(QtSignalToDelegate &&rh)
    : QObject(), connection(std::move(rh.connection)), v(std::move(rh.v))
  {}
  QtSignalToDelegate(const QObject *pSourceObj, const QMetaMethod &m, const Variant::VarDelegate &d) : v(d)
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

  explicit operator bool() { return connection; }

private:
  QMetaMethod lookupSignalHandler(const QMetaMethod &m);

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
  Variant v;
};

} // namespace qt

#endif  // SIGNALTODELEGATE_H
