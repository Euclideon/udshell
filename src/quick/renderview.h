#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <QtQuick/QQuickFramebufferObject>

class FboRenderer;

class RenderView : public QQuickFramebufferObject
{
  Q_OBJECT

public:
  RenderView(QQuickItem *pParent = 0);
  virtual ~RenderView();

  Renderer *createRenderer() const;

private:
  // TODO: Avoid this crap
  friend class FboRenderer;

  void componentComplete();

  QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData);
};


#endif  // RENDERVIEW_H
