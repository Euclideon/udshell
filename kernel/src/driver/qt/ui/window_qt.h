#ifndef WINDOW_QT_H
#define WINDOW_QT_H

#include "../epkernel_qt.h"
#include "ep/cpp/component/commandmanager.h"

namespace qt
{

class QtWindowEventFilter : public QObject
{
  Q_OBJECT

public:
  QtWindowEventFilter(QQuickWindow *pParent) : QObject(pParent) {}

protected:
  bool eventFilter(QObject *pObj, QEvent *pEvent);
};

} // namespace qt

#endif  // WINDOW_QT_H
