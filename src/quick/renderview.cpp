#include "renderview.h"
#include "udKernel.h"

#include <QtQuick/QQuickWindow>
#include <QtGui/QOpenGLFramebufferObject>

class FboRenderer : public QQuickFramebufferObject::Renderer
{
public:
  FboRenderer(const QQuickFramebufferObject *item) : m_item(item)
  {
    udDebugPrintf("\nFboRenderer::FboRenderer()\n");
  }

  void render()
  {
    udDebugPrintf("\nFboRenderer::render()\n");
    m_item->window()->resetOpenGLState();
  }

  QOpenGLFramebufferObject *createFramebufferObject(const QSize &size)
  {
    udDebugPrintf("\nFboRenderer::createFramebufferObject()\n");
    // TODO: Set up appropriate format
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
  }

  void synchronize(QQuickFramebufferObject * item)
  {
    udDebugPrintf("\nFboRenderer::synchronize()\n");
  }

  const QQuickFramebufferObject *m_item;
};


RenderView::RenderView(QQuickItem *pParent)
  : QQuickFramebufferObject(pParent)
{
  udDebugPrintf("RenderView::RenderView()\n");
}

RenderView::~RenderView()
{
  udDebugPrintf("RenderView::~RenderView()\n");
}

QQuickFramebufferObject::Renderer *RenderView::createRenderer() const
{
  udDebugPrintf("RenderView::createRenderer()\n");
  return new FboRenderer(this);
}

void RenderView::componentComplete()
{
  udDebugPrintf("RenderView::componentComplete()\n");
  QQuickFramebufferObject::componentComplete();

  // TODO: Focus should be set based on mouse click from main window - possibly handle in QML
  // TODO: Accepted mouse buttons should be item specific
  setFocus(true);
  setAcceptedMouseButtons(Qt::LeftButton);
}
