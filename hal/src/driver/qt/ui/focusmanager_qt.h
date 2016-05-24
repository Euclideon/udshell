#ifndef FOCUSMANAGER_QT_H
#define FOCUSMANAGER_QT_H

#include "ep/cpp/platform.h"

#include <QQuickWindow>
#include <QList>
#include <QMap>

namespace qt {

class QtFocusManager : public QObject
{
  Q_OBJECT

public:
  ~QtFocusManager();

  Q_INVOKABLE void push(QQuickItem *pQtItem);
  Q_INVOKABLE void pushActiveFocusItem(QQuickWindow *pQtWindow = nullptr);
  Q_INVOKABLE void setFocus(QQuickItem *pQtItem);
  Q_INVOKABLE QQuickItem *pop(QQuickWindow *pQtWindow = nullptr);
  Q_INVOKABLE QQuickItem *restoreFocus(QQuickWindow *pQtWindow = nullptr);
  Q_INVOKABLE void clear(QQuickWindow *pQtWindow = nullptr);

protected slots:
  void OnItemDestroyed(QObject *obj);
  void OnItemWindowChanged(QQuickWindow *pQtWindow);
  void OnItemParentChanged(QQuickItem *pQtItem);
  void OnItemActiveFocusChanged(bool activeFocus);
  void OnWindowVisibleChanged(bool visible);
  void OnWindowFocusItemChanged();
  void OnFocusItemVisibleChanged();
  void OnFocusItemDestroyed(QObject *obj);
  void OnFocusItemActiveFocusChanged(bool activeFocus);

protected:
  void ActiveFocusLost(QQuickItem *pQtItem);
  void ActiveFocusGained(QQuickItem *pQtItem);
  void ConnectItemSignals(QQuickItem *pQtItem);
  void DisconnectItemSignals(QQuickItem *pQtItem);
  void CreateWindowStack(QQuickWindow *pQtWindow);
  void DestroyWindowStack(QQuickWindow *pQtWindow);

  QMap<QQuickWindow *, QQuickItem *> activeFocusMap;
  QMap<QQuickWindow *, QList<QQuickItem *>> stackMap;
};

} // end namespace qt

#endif // FOCUSMANAGER_QT_H
