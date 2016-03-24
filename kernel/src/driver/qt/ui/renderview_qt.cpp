#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "driver/qt/epqt.h"

#include <QtGui/QOpenGLFramebufferObject>
#include <QSGSimpleTextureNode>
#include <QOpenGLDebugLogger>

#include "synchronisedptr.h"
#include "../epkernel_qt.h"
#include "renderview_qt.h"
#include "components/viewimpl.h"
#include "renderscene.h"

epKeyCode qtKeyToEPKey(Qt::Key qk);

namespace qt {

class QtFboRenderer : public QQuickFramebufferObject::Renderer
{
public:
  QtFboRenderer(const QQuickFramebufferObject *item) : m_item(item) { }

  void render()
  {
    if (spRenderableView)
      spRenderableView->RenderGPU();

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
    QtRenderView *pQtRenderView = (QtRenderView*)item;

    if (pQtRenderView->dirty)
    {
      spRenderableView = ep::SynchronisedPtr<ep::RenderableView>(pQtRenderView->spView->GetImpl<ep::ViewImpl>()->GetRenderableView(), QtApplication::Kernel());
      pQtRenderView->dirty = false;
    }
  }

  const QQuickFramebufferObject *m_item;
  ep::SynchronisedPtr<ep::RenderableView> spRenderableView;
};


QtRenderView::QtRenderView(QQuickItem *pParent)
  : QQuickFramebufferObject(pParent)
  , spView(nullptr)
  , dirty(false)
{
  QtApplication::Kernel()->LogTrace("Create RenderView Quick Item");

  // handle resize
  QObject::connect(this, &QQuickItem::widthChanged, this, &QtRenderView::OnResize);
  QObject::connect(this, &QQuickItem::heightChanged, this, &QtRenderView::OnResize);

  QObject::connect(this, &QQuickItem::visibleChanged, this, &QtRenderView::OnVisibleChanged);
}

QtRenderView::~QtRenderView()
{
  QtApplication::Kernel()->LogTrace("Destroy RenderView Quick Item");

  if (spView)
    spView->FrameReady.Unsubscribe(ep::Delegate<void()>(this, &QtRenderView::OnFrameReady));
}

QQuickFramebufferObject::Renderer *QtRenderView::createRenderer() const
{
  QtApplication::Kernel()->LogTrace("Create RenderView Renderer");
  return new QtFboRenderer(this);
}

void QtRenderView::AttachView(ep::ViewRef _spView)
{
  QtApplication::Kernel()->LogTrace("RenderView::AttachView()");

  spView = _spView;
  spView->FrameReady.Subscribe(ep::Delegate<void()>(this, &QtRenderView::OnFrameReady));

  // TEMP HAX:
  QtApplication::Kernel()->SetFocusView(spView);
}

void QtRenderView::componentComplete()
{
  QtApplication::Kernel()->LogTrace("RenderView::componentComplete()");
  QQuickFramebufferObject::componentComplete();

  // TODO: Focus should be set based on mouse click from main window - possibly handle in QML
  // TODO: Accepted mouse buttons should be item specific
  setFocus(true);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
}

QSGNode *QtRenderView::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData)
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

void QtRenderView::keyPressEvent(QKeyEvent *pEv)
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
  if (spView && spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
    pEv->accept();
}

void QtRenderView::keyReleaseEvent(QKeyEvent *pEv)
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
  if (spView && spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
    pEv->accept();
}

void QtRenderView::mouseDoubleClickEvent(QMouseEvent *pEv)
{
  bool handled = false;
  // translate and process
  if (handled)
    pEv->accept();
}

// TODO: MASSIVE HAX!!! FIX ME!!
static qreal mouseLastX, mouseLastY;

void QtRenderView::mouseMoveEvent(QMouseEvent *pEv)
{
  auto pos = pEv->localPos();
  qreal x = pos.x();
  qreal y = pos.y();

  epInputEvent ev;
  ev.deviceType = epID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = epInputEvent::Move;
  ev.move.xDelta = (float)(mouseLastX - x);
  ev.move.yDelta = (float)(mouseLastY - y);
  ev.move.xAbsolute = (float)x;
  ev.move.yAbsolute = (float)y;
  if (spView && spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
    pEv->accept();

  mouseLastX = x;
  mouseLastY = y;
}

void QtRenderView::mousePressEvent(QMouseEvent *pEv)
{
  if ((pEv->buttons() & Qt::LeftButton) && !hasActiveFocus())
    forceActiveFocus();

  epInputEvent ev;
  ev.deviceType = epID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = epInputEvent::Key;
  ev.key.key = pEv->button();
  ev.key.state = 1;
  if (spView && spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
    pEv->accept();
}

void QtRenderView::mouseReleaseEvent(QMouseEvent *pEv)
{
  epInputEvent ev;
  ev.deviceType = epID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = epInputEvent::Key;
  ev.key.key = pEv->button();
  ev.key.state = 0;
  if (spView && spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
    pEv->accept();
}

void QtRenderView::touchEvent(QTouchEvent *pEv)
{
  bool handled = false;
  // translate and process
  if (handled)
    pEv->accept();
}

void QtRenderView::hoverMoveEvent(QHoverEvent *pEv)
{
  auto pos = pEv->pos();
  qreal x = pos.x();
  qreal y = pos.y();

  auto oldPos = pEv->oldPos();
  mouseLastX = oldPos.x();
  mouseLastY = oldPos.y();

  epInputEvent ev;
  ev.deviceType = epID_Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = epInputEvent::Move;
  ev.move.xDelta = (float)(mouseLastX - x);
  ev.move.yDelta = (float)(mouseLastY - y);
  ev.move.xAbsolute = (float)x;
  ev.move.yAbsolute = (float)y;

  if (spView)
    spView->GetImpl<ep::ViewImpl>()->InputEvent(ev);
}

} // namespace qt

#else
EPEMPTYFILE
#endif  // EPUI_DRIVER == EPDRIVER_QT
