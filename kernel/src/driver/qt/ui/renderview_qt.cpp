#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

// suppress warnings from qt
#if defined(_MSC_VER)
# pragma warning(push,3)
#endif
#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickFramebufferObject>
#include <QtGui/QOpenGLFramebufferObject>
#include <QSGSimpleTextureNode>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif

#include "../epkernel_qt.h"
#include "renderview_qt.h"
#include "renderscene.h"

epKeyCode qtKeyToEPKey(Qt::Key qk);

namespace qt {

class FboRenderer : public QQuickFramebufferObject::Renderer
{
public:
  FboRenderer(const QQuickFramebufferObject *item) : m_item(item) { }

  void render()
  {
    if (spRenderView)
      spRenderView->RenderGPU();

    if (m_item->window())
        m_item->window()->resetOpenGLState();
  }

  QOpenGLFramebufferObject *createFramebufferObject(const QSize &size)
  {
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

    if (pRenderView->dirty)
    {
      spRenderView = pRenderView->spView->GetRenderableView();
      pRenderView->dirty = false;
    }
  }

  const QQuickFramebufferObject *m_item;
  kernel::RenderableViewRef spRenderView = nullptr;
};


RenderView::RenderView(QQuickItem *pParent)
  : QQuickFramebufferObject(pParent)
  , spView(nullptr)
  , dirty(false)
{
  QtApplication::Kernel()->LogTrace("Create RenderView Quick Item");

  // handle resize
  QObject::connect(this, &QQuickItem::widthChanged, this, &RenderView::OnResize);
  QObject::connect(this, &QQuickItem::heightChanged, this, &RenderView::OnResize);

  QObject::connect(this, &QQuickItem::visibleChanged, this, &RenderView::OnVisibleChanged);
}

RenderView::~RenderView()
{
  QtApplication::Kernel()->LogTrace("Destroy RenderView Quick Item");

  if (spView)
    spView->FrameReady.Unsubscribe(Delegate<void()>(this, &RenderView::OnFrameReady));
}

QQuickFramebufferObject::Renderer *RenderView::createRenderer() const
{
  QtApplication::Kernel()->LogTrace("Create RenderView Renderer");
  return new FboRenderer(this);
}

void RenderView::AttachView(kernel::ViewRef _spView)
{
  QtApplication::Kernel()->LogTrace("RenderView::AttachView()");

  spView = _spView;
  spView->FrameReady.Subscribe(Delegate<void()>(this, &RenderView::OnFrameReady));

  // TEMP HAX:
  QtApplication::Kernel()->SetFocusView(spView);
}

void RenderView::componentComplete()
{
  QtApplication::Kernel()->LogTrace("RenderView::componentComplete()");
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
  epKeyCode kc = qtKeyToEPKey((Qt::Key)pEv->key());
  if (kc == epKC_Unknown)
    return;
  epInputEvent ev;
  ev.deviceType = epID_Keyboard;
  ev.deviceId = 0; // TODO: get keyboard id
  ev.eventType = epInputEvent::Key;
  ev.key.key = kc;
  ev.key.state = 1;
  if (spView && spView->InputEvent(ev))
    pEv->accept();
}

void RenderView::keyReleaseEvent(QKeyEvent *pEv)
{
  epKeyCode kc = qtKeyToEPKey((Qt::Key)pEv->key());
  if (kc == epKC_Unknown)
    return;
  epInputEvent ev;
  ev.deviceType = epID_Keyboard;
  ev.deviceId = 0; // TODO: get keyboard id
  ev.eventType = epInputEvent::Key;
  ev.key.key = kc;
  ev.key.state = 0;
  if (spView && spView->InputEvent(ev))
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

  epInputEvent ev;
  ev.deviceType = epID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = epInputEvent::Move;
  ev.move.xDelta = (float)(lastX - x); // TODO: MASSIVE HAX!!! FIX ME!!
  ev.move.yDelta = (float)(lastY - y); // TODO: MASSIVE HAX!!! FIX ME!!
  ev.move.xAbsolute = (float)x;
  ev.move.yAbsolute = (float)y;
  if (spView && spView->InputEvent(ev))
    pEv->accept();

  lastX = x; // TODO: MASSIVE HAX!!! FIX ME!!
  lastY = y;
}

void RenderView::mousePressEvent(QMouseEvent *pEv)
{
  epInputEvent ev;
  ev.deviceType = epID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = epInputEvent::Key;
  ev.key.key = pEv->button();
  ev.key.state = 1;
  if (spView && spView->InputEvent(ev))
    pEv->accept();
}

void RenderView::mouseReleaseEvent(QMouseEvent *pEv)
{
  epInputEvent ev;
  ev.deviceType = epID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = epInputEvent::Key;
  ev.key.key = pEv->button();
  ev.key.state = 0;
  if (spView && spView->InputEvent(ev))
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

#else
EPEMPTYFILE
#endif  // EPUI_DRIVER == EPDRIVER_QT
