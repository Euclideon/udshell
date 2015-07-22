#include "renderview.h"
#include "udKernel.h"
#include "udView.h"

#include <QtQuick/QQuickWindow>
#include <QtGui/QOpenGLFramebufferObject>


extern udKernel *s_pKernel;

class FboRenderer : public QQuickFramebufferObject::Renderer
{
public:
  FboRenderer(const QQuickFramebufferObject *item) : m_item(item), dirty(true)
  {
    udDebugPrintf("FboRenderer::FboRenderer()\n");
  }

  void render()
  {
    udDebugPrintf("FboRenderer::render()\n");
    UDASSERT(s_pKernel->GetFocusView(), "No focus view");

    if (dirty)
    {
      udDebugPrintf("DIRTY!\n");
      s_pKernel->GetFocusView()->Resize(framebufferObject()->width(), framebufferObject()->height());
      dirty = false;
    }

    s_pKernel->GetFocusView()->Render();
    m_item->window()->resetOpenGLState();
  }

  QOpenGLFramebufferObject *createFramebufferObject(const QSize &size)
  {
    udDebugPrintf("FboRenderer::createFramebufferObject()\n");
    // TODO: Set up appropriate format
    QOpenGLFramebufferObjectFormat format;
    //format.setInternalTextureFormat(GL_RGBA8);
    //format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    //format.setSamples(4);
    dirty = true;
    return new QOpenGLFramebufferObject(size, format);
  }

  void synchronize(QQuickFramebufferObject * item)
  {
    udDebugPrintf("FboRenderer::synchronize()\n");
  }

  const QQuickFramebufferObject *m_item;
  bool dirty;
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
