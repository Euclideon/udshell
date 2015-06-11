#include "RenderView.h"

#include "udViewer.h"

RenderView::RenderView(QQuickItem *pParent)
  : QQuickFramebufferObject(pParent)
{
}

RenderView::~RenderView()
{
}

QQuickFramebufferObject::Renderer *RenderView::createRenderer() const
{
  return new Renderer;
}

void RenderView::Renderer::render()
{

}
