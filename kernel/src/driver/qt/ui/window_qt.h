#ifndef WINDOW_QT_H
#define WINDOW_QT_H

#include "../epkernel_qt.h"
#include "components/commandmanager.h"

namespace qt
{

class QtWindowEventFilter : public QObject
{
  Q_OBJECT

public:
  QtWindowEventFilter(QObject *pParent) : QObject(pParent) {}

protected:
  bool eventFilter(QObject *pObj, QEvent *pEvent);
};

// TODO: remove this when we know for sure we don't need it
/*class QtWindow : public QQuickWindow
{
  Q_OBJECT

public:
  QtWindow(QWindow *pParent = 0) : QQuickWindow(pParent) {}
  virtual ~QtWindow() {}

protected:
  void resizeEvent(QResizeEvent *pEv)
  {
    QQuickWindow::resizeEvent(pEv);
    QQuickItem *pRoot = contentItem();
    pRoot->setSize(pEv->size());
  }

  void keyPressEvent(QKeyEvent *pEv)
  {
    if (!pEv->isAutoRepeat())
    {
      QKeySequence seq(pEv->key() + pEv->modifiers());
      bool ret = QtApplication::Kernel()->GetCommandManager()->HandleShortcutEvent(seq.toString().toUtf8().data());
      if (ret)
        return;
    }

    QQuickWindow::keyPressEvent(pEv);
  }
};*/

} // namespace qt

#endif  // WINDOW_QT_H
