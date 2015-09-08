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

udKeyCode qtKeyToUDKey(Qt::Key qk);

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
    UDASSERT(spRenderView, "No view");

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
//  spView = s_pKernel->CreateComponent<ud::View>();
  spView = s_pKernel->GetFocusView();
  spView->FrameReady.Subscribe(udDelegate<void()>(this, &RenderView::OnFrameReady));

  s_pKernel->SetFocusView(spView);
}

RenderView::~RenderView()
{
  udDebugPrintf("RenderView::~RenderView()\n");

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

void RenderView::keyPressEvent(QKeyEvent *pEv)
{
  udKeyCode kc = qtKeyToUDKey((Qt::Key)pEv->key());
  if (kc == udKC_Unknown)
    return;
  udInputEvent ev;
  ev.deviceType = udID_Keyboard;
  ev.deviceId = 0; // TODO: get keyboard id
  ev.eventType = udInputEvent::Key;
  ev.key.key = kc;
  ev.key.state = 1;
  if(spView->InputEvent(ev))
    pEv->accept();
}
void RenderView::keyReleaseEvent(QKeyEvent *pEv)
{
  udKeyCode kc = qtKeyToUDKey((Qt::Key)pEv->key());
  if (kc == udKC_Unknown)
    return;
  udInputEvent ev;
  ev.deviceType = udID_Keyboard;
  ev.deviceId = 0; // TODO: get keyboard id
  ev.eventType = udInputEvent::Key;
  ev.key.key = kc;
  ev.key.state = 0;
  if (spView->InputEvent(ev))
    pEv->accept();
}
void RenderView::mouseDoubleClickEvent(QMouseEvent *pEv)
{
  bool handled = false;
  // translate and process
  if (handled)
    pEv->accept();
}
void RenderView::mouseMoveEvent(QMouseEvent *pEv)
{
  auto pos = pEv->localPos();
  qreal x = pos.x();
  qreal y = pos.y();

  // TODO: MASSIVE HAX!!! FIX ME!!
  static qreal lastX = -100000, lastY = -100000;
  if (lastX == -100000 && lastY == -100000)
  {
    lastX = x;
    lastY = y;
  }

  udInputEvent ev;
  ev.deviceType = udID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = udInputEvent::Move;
  ev.move.xDelta = (float)(lastX - x); // TODO: MASSIVE HAX!!! FIX ME!!
  ev.move.yDelta = (float)(lastY - y); // TODO: MASSIVE HAX!!! FIX ME!!
  ev.move.xAbsolute = (float)x;
  ev.move.yAbsolute = (float)y;
  if (spView->InputEvent(ev))
    pEv->accept();

  lastX = x; // TODO: MASSIVE HAX!!! FIX ME!!
  lastY = y;
}
void RenderView::mousePressEvent(QMouseEvent *pEv)
{
  udInputEvent ev;
  ev.deviceType = udID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = udInputEvent::Key;
  ev.key.key = pEv->button();
  ev.key.state = 1;
  if (spView->InputEvent(ev))
    pEv->accept();
}
void RenderView::mouseReleaseEvent(QMouseEvent *pEv)
{
  udInputEvent ev;
  ev.deviceType = udID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = udInputEvent::Key;
  ev.key.key = pEv->button();
  ev.key.state = 0;
  if (spView->InputEvent(ev))
    pEv->accept();
}
void RenderView::touchEvent(QTouchEvent *pEv)
{
  bool handled = false;
  // translate and process
  if (handled)
    pEv->accept();
}

} // namespace qt

#endif  // UDUI_DRIVER == UDDRIVER_QT
