#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "driver/qt/epqt.h"

#include "ep/cpp/component/view.h"

#include <QtQuick/QQuickFramebufferObject>


namespace qt {

class FboRenderer;

class RenderView : public QQuickFramebufferObject
{
  Q_OBJECT

public:
  RenderView(QQuickItem *pParent = 0);
  virtual ~RenderView();

  Renderer *createRenderer() const override;

  void AttachView(ViewRef spView);

private:
  ViewRef spView;
  bool dirty;

  // TODO: Avoid this crap
  friend class FboRenderer;

  void componentComplete() override;

  QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData) override;

  void keyPressEvent(QKeyEvent * event) override;
  void keyReleaseEvent(QKeyEvent * event) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void touchEvent(QTouchEvent *) override;
  void hoverMoveEvent(QHoverEvent *) override;
//  void inputMethodEvent(QInputMethodEvent *) override;

private slots:
  void OnResize()
  {
    int w = (int)width();
    int h = (int)height();

    if (spView && w > 0 && h > 0)
      spView->Resize(w, h);
  }

  void OnFrameReady()
  {
    dirty = true;
    update();
  }

  void OnVisibleChanged()
  {
    if (spView)
    {
      if (isVisible())
        spView->Activate();
      else
        spView->Deactivate();
    }
  }
};

} // namespace qt

#endif  // RENDERVIEW_H
