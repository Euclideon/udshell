#ifndef WINDOW_QT_H
#define WINDOW_QT_H

#include <QQuickWindow>
#include <QQuickItem>
#include "kernel.h"
#include "components/shortcutmanager.h"

extern ud::Kernel *s_pKernel;

namespace qt
{

class QtWindow : public QQuickWindow
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
      bool ret = s_pKernel->GetShortcutManager()->HandleShortcutEvent(seq.toString().toUtf8().data());
      if (ret)
        return;
    }

    QQuickWindow::keyPressEvent(pEv);
  }
};

} // namespace qt

#endif  // WINDOW_QT_H
