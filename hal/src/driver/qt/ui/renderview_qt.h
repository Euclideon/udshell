#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "driver/qt/epqt.h"

#include "ep/cpp/component/view.h"

#include <QtQuick/QQuickFramebufferObject>


namespace qt {

class QtFboRenderer;

class QtRenderView : public QQuickFramebufferObject
{
  Q_OBJECT

public:
  QtRenderView(QQuickItem *pParent = 0);
  virtual ~QtRenderView();

  Renderer *createRenderer() const override;

  void AttachView(ep::ViewRef spView);

private slots:
  void OnResize();
  void OnVisibleChanged();

private:
  // TODO: Avoid this crap
  friend class QtFboRenderer;

  void componentComplete() override;

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
  QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData) override;
#endif

  void focusInEvent(QFocusEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
  void keyReleaseEvent(QKeyEvent *) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void touchEvent(QTouchEvent *) override;
  void hoverMoveEvent(QHoverEvent *) override;
  void wheelEvent(QWheelEvent *) override;

  void OnFrameReady()
  {
    dirty = true;
    update();
  }

  ep::ViewRef spView = nullptr;
  bool dirty = false;
  qreal mouseLastX = 0.0f;
  qreal mouseLastY = 0.0f;
};

} // namespace qt

#endif  // RENDERVIEW_H
