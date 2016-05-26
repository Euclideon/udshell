#include "driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "window_qt.h"

#include <QQuickWindow>
#include <QQuickItem>

namespace qt {

bool QtWindowEventFilter::eventFilter(QObject *pObj, QEvent *pEvent)
{
  QQuickWindow *pWindow = qobject_cast<QQuickWindow*>(pObj);
  EPASSERT(pWindow != nullptr, "QtWindowEventFilter must be bound to a QQuickWindow");

  switch (pEvent->type())
  {
    case QEvent::Resize:
    {
      QResizeEvent *pResizeEvent = static_cast<QResizeEvent*>(pEvent);
      QQuickItem *pRoot = pWindow->contentItem();
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
    case QEvent::Close:
    {
      QtApplication::Kernel()->UnregisterWindow(pWindow);
      break;
    }
    default:
      break;
  }

  // Pass the event forward
  return false;
}

} // namespace qt

#else
EPEMPTYFILE
#endif  // EPUI_DRIVER == EPDRIVER_QT
