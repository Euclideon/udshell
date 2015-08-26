#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

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

namespace qt
{

class FboRenderer : public QQuickFramebufferObject::Renderer
{
public:
  FboRenderer(const QQuickFramebufferObject *item) : m_item(item)
  {
    udDebugPrintf("FboRenderer::FboRenderer()\n");
  }

  void render()
  {
    UDASSERT(s_pKernel->GetFocusView(), "No focus view");

    if (spRenderView)
      spRenderView->RenderGPU();

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
    return new QOpenGLFramebufferObject(size, format);
  }

  void synchronize(QQuickFramebufferObject * item)
  {
    RenderView *pRenderView = (RenderView*)item;

    udDebugPrintf("FboRenderer::synchronize()\n");

    if (pRenderView->dirty)
    {
      ud::ViewRef spView = s_pKernel->GetFocusView();
      spRenderView = spView->GetRenderableView();

      update();

      pRenderView->dirty = false;
    }
  }

  const QQuickFramebufferObject *m_item;
  ud::RenderableViewRef spRenderView = nullptr;
};


RenderView::RenderView(QQuickItem *pParent)
  : QQuickFramebufferObject(pParent), dirty(false)
{
  udDebugPrintf("RenderView::RenderView()\n");

  // handle resize
  QObject::connect(this, &QQuickItem::widthChanged, this, &RenderView::OnResize);
  QObject::connect(this, &QQuickItem::heightChanged, this, &RenderView::OnResize);

  // HAX HAX!
  ud::ViewRef spView = s_pKernel->GetFocusView();
  spView->FrameReady.Subscribe(udDelegate<void()>(this, &RenderView::OnFrameReady));
}

RenderView::~RenderView()
{
  udDebugPrintf("RenderView::~RenderView()\n");

  ud::ViewRef spView = s_pKernel->GetFocusView();
  spView->FrameReady.Unsubscribe(udDelegate<void()>(this, &RenderView::OnFrameReady));
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

} // namespace qt

#endif  // UDUI_DRIVER == UDDRIVER_QT
