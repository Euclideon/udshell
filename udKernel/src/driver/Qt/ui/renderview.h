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

private:
  bool dirty;

  // TODO: Avoid this crap
  friend class FboRenderer;

  void componentComplete();

  QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData);

private slots:
  void OnResize()
  {
    ud::ViewRef spView = s_pKernel->GetFocusView();
    spView->Resize(width(), height());
  }

  void OnFrameReady()
  {
    dirty = true;
  }
};

} // namespace qt

#endif  // RENDERVIEW_H
