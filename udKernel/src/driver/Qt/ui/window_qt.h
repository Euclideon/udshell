#ifndef WINDOW_QT_H
#define WINDOW_QT_H

#include <QQuickWindow>
#include <QQuickItem>

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
};

} // namespace qt

#endif  // WINDOW_QT_H
