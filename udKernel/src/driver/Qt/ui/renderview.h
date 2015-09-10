#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <QtQuick/QQuickFramebufferObject>

#include "kernel.h"
#include "renderscene.h"
#include "components/view.h"

extern ud::Kernel *s_pKernel;

namespace qt
{

class FboRenderer;

class RenderView : public QQuickFramebufferObject
{
  Q_OBJECT

public:
  RenderView(QQuickItem *pParent = 0);
  virtual ~RenderView();

  Renderer *createRenderer() const;

  void AttachView(ud::ViewRef spView);

private:
  ud::ViewRef spView;
  bool dirty;

  // TODO: Avoid this crap
  friend class FboRenderer;

  void componentComplete();

  QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData);

  void keyPressEvent(QKeyEvent * event) override;
  void keyReleaseEvent(QKeyEvent * event) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void touchEvent(QTouchEvent *) override;
//  void inputMethodEvent(QInputMethodEvent *) override;

private slots:
  void OnResize()
  {
    if (spView)
      spView->Resize(width(), height());
  }

  void OnFrameReady()
  {
    dirty = true;
    update();
  }
};

} // namespace qt

#endif  // RENDERVIEW_H
