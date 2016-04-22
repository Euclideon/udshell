#pragma once
#ifndef SIGNALTODELEGATE_H
#define SIGNALTODELEGATE_H

#include "driver/qt/epqt.h"
#include "driver/qt/util/typeconvert_qt.h"

#include <QObject>
#include <QMetaMethod>

namespace qt {

// This class will connect to a Qt signal of any type and redirect it back to its associated Euclideon Platform Event
// Since the Event class tracks the list of subscribers, this essentially allows generic delegates to be connected to
// Qt signals as if they were general EP Events.
// NOTE: This requires some MOC magic - in order to allow a generic signal handler, we need to pipe signals of all
// types via the execute() method.
class QtSignalMapper : public QObject
{
  // NOTE: this prevents MOC from generating anything for our object - replace this if we need to regenerate
  Q_OBJECT_FAKE

public:
  QtSignalMapper(const QMetaMethod &sigMethod) : QObject(nullptr), signal(sigMethod) {}
  ~QtSignalMapper() {}

  ep::SubscriptionRef Subscribe(QObject *pSourceObj, const ep::VarDelegate &del);

private:
  Q_DISABLE_COPY(QtSignalMapper)

  // NOTE: the connected Qt signal gets redirected to here
  void execute(void **args);

private slots:
  //void execute() {}   // !! UNCOMMENT THIS IF WE NEED TO RE-MOC!

  void onInstanceDestroyed(QObject *pObj) { instanceMap.Remove(pObj); }

private:
  // This struct represents a connection instance - we need one connection per specific QObject
  struct QtConnection
  {
    ~QtConnection() { QObject::disconnect(connection); }
    ep::VarEvent event;
    QMetaObject::Connection connection;
  };

  ep::AVLTree<QObject *, QtConnection> instanceMap;
  QMetaMethod signal;
};

} // namespace qt

#endif  // SIGNALTODELEGATE_H
