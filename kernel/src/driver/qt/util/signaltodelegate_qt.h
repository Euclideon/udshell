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
    epDelegate<void()> del = v.as<epDelegate<void()>>(); \
    del(); \
  }

#define QT_SIGNAL_HANDLER_1(TYPE1) \
  void SignalHandler(TYPE1 arg1) { \
    epDelegate<void(epVariant)> del = v.as<epDelegate<void(epVariant)>>(); \
    del(epVariant(arg1)); \
  }

#define QT_SIGNAL_HANDLER_2(TYPE1, TYPE2) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2) { \
    epDelegate<void(epVariant, epVariant)> del = v.as<epDelegate<void(epVariant,epVariant)>>(); \
    del(epVariant(arg1), epVariant(arg2)); \
  }

#define QT_SIGNAL_HANDLER_3(TYPE1, TYPE2, TYPE3) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2, TYPE3 arg3) { \
    epDelegate<void(epVariant,epVariant,epVariant)> del = v.as<epDelegate<void(epVariant,epVariant,epVariant)>>(); \
    del(epVariant(arg1), epVariant(arg2), epVariant(arg3)); \
  }

#define QT_SIGNAL_HANDLER_4(TYPE1, TYPE2, TYPE3, TYPE4) \
  void SignalHandler(TYPE1 arg1, TYPE2 arg2, TYPE3 arg3, TYPE4 arg4) { \
    epDelegate<void(epVariant,epVariant,epVariant,epVariant)> del = v.as<epDelegate<void(epVariant,epVariant,epVariant,epVariant)>>(); \
    del(epVariant(arg1), epVariant(arg2), epVariant(arg3), epVariant(arg4)); \
  }


class QtSignalToDelegate : public QObject
{
  Q_OBJECT

public:
  QtSignalToDelegate(const QObject *pSourceObj, const QMetaMethod &m, const epVariant::VarDelegate &d) : v(d)
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
  epVariant v;
};

} // namespace qt

#endif  // SIGNALTODELEGATE_H
