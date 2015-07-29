// Warning from QSGGeometry
#if _MSC_VER
# pragma warning(disable: 4512)
#endif

#include <QtQuick/QQuickWindow>
#include <QtGui/QOpenGLFramebufferObject>
#include <QSGSimpleTextureNode>

#include "kernel.h"
#include "renderview.h"
#include "components/view.h"

extern ud::Kernel *s_pKernel;


class FboRenderer : public QQuickFramebufferObject::Renderer
{
public:
  FboRenderer(const QQuickFramebufferObject *item) : m_item(item), dirty(true)
  {
    udDebugPrintf("FboRenderer::FboRenderer()\n");
  }

  void render()
  {
    UDASSERT(s_pKernel->GetFocusView(), "No focus view");

    if (dirty)
    {
      udDebugPrintf("DIRTY!\n");
      s_pKernel->GetFocusView()->Resize(framebufferObject()->width(), framebufferObject()->height());
      dirty = false;
    }

    s_pKernel->GetFocusView()->Render();
    m_item->window()->resetOpenGLState();

    // TODO: Remove this once we have proper dirty state being set
    update();
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

QSGNode *RenderView::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData)
{
  // MAD HAX: QML flips our framebuffer texture along the y-axis - better fix in 5.6: https://bugreports.qt.io/browse/QTBUG-41073
  if (!node)
  {
    node = QQuickFramebufferObject::updatePaintNode(node, nodeData);
    QSGSimpleTextureNode *n = static_cast<QSGSimpleTextureNode *>(node);
    if (n)
      n->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    return node;
  }
  return QQuickFramebufferObject::updatePaintNode(node, nodeData);
}
