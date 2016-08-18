#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include "driver/qt/epqt.h"
#include "driver/qt/util/typeconvert_qt.h"

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

  Q_PROPERTY(QVariant view READ getView WRITE setView)

  Renderer *createRenderer() const override;

  QVariant getView() const { return ep::Variant(spView).as<QVariant>(); }
  void setView(const QVariant &view);

private slots:
  void onResize();
  void onVisibleChanged();

private:
  // TODO: Avoid this crap
  friend class QtFboRenderer;

  void componentComplete() override;

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

  void onFrameReady()
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
