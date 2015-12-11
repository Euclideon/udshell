#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include <QQuickWindow>
#include <QQuickItem>

#include "window_qt.h"

namespace qt {

bool QtWindowEventFilter::eventFilter(QObject *pObj, QEvent *pEvent)
{
  switch (pEvent->type())
  {
  case QEvent::Resize:
  {
    QResizeEvent *pResizeEvent = static_cast<QResizeEvent*>(pEvent);
    QQuickItem *pRoot = static_cast<QQuickWindow*>(pObj)->contentItem();
    pRoot->setSize(pResizeEvent->size());
    break;
  }
  case QEvent::KeyPress:
  {
    QKeyEvent *pKeyEvent = static_cast<QKeyEvent*>(pEvent);
    if (!pKeyEvent->isAutoRepeat())
    {
      QKeySequence seq(pKeyEvent->key() + pKeyEvent->modifiers());
      return QtApplication::Kernel()->GetCommandManager()->HandleShortcutEvent(seq.toString().toUtf8().data());
    }
    break;
  }
  default:
    return QObject::eventFilter(pObj, pEvent);
  }

  // Pass the event forward
  return false;
}

} // namespace qt

#else
EPEMPTYFILE
#endif  // EPUI_DRIVER == EPDRIVER_QT
