#include "driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "driver/qt/epqt.h"

#include <QtGui/QOpenGLFramebufferObject>
#include <QSGSimpleTextureNode>
#include <QOpenGLDebugLogger>

#include "synchronisedptr.h"
#include "renderscene.h"
#include "components/viewimpl.h"

#include "driver/qt/epkernel_qt.h"
#include "driver/qt/ui/renderview_qt.h"

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
{
  QObject::connect(this, &QQuickItem::widthChanged, this, &QtRenderView::OnResize);
  QObject::connect(this, &QQuickItem::heightChanged, this, &QtRenderView::OnResize);
  QObject::connect(this, &QQuickItem::visibleChanged, this, &QtRenderView::OnVisibleChanged);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  setMirrorVertically(true);
#endif
}

QtRenderView::~QtRenderView()
{
  if (spView)
    spView->FrameReady.Unsubscribe(ep::Delegate<void()>(this, &QtRenderView::OnFrameReady));
}

QQuickFramebufferObject::Renderer *QtRenderView::createRenderer() const
{
  return new QtFboRenderer(this);
}

void QtRenderView::AttachView(ep::ViewRef _spView)
{
  spView = _spView;
  spView->FrameReady.Subscribe(ep::Delegate<void()>(this, &QtRenderView::OnFrameReady));

  // TEMP HAX:
  QtApplication::Kernel()->SetFocusView(spView);
}

void QtRenderView::OnResize()
{
  int w = (int)width();
  int h = (int)height();

  if (spView && w > 0 && h > 0)
    spView->Resize(w, h);
}

void QtRenderView::OnVisibleChanged()
{
  if (spView)
  {
    if (isVisible())
      spView->Activate();
    else
      spView->Deactivate();
  }
}

void QtRenderView::componentComplete()
{
  QQuickFramebufferObject::componentComplete();

  // TODO: Focus should be set based on mouse click from main window - possibly handle in QML
  setFocus(true);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0))
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
#endif

void QtRenderView::focusInEvent(QFocusEvent * event)
{
  epInputEvent ev;
  ev.deviceType = epID_Keyboard;
  ev.deviceId = 0;
  ev.eventType = epInputEvent::Focus;
  ev.hasFocus = true;

  if (spView && spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
    event->accept();
  else
    event->ignore();
}

void QtRenderView::focusOutEvent(QFocusEvent * event)
{
  epInputEvent ev;
  ev.deviceType = epID_Keyboard;
  ev.deviceId = 0;
  ev.eventType = epInputEvent::Focus;
  ev.hasFocus = false;

  if (spView && spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
    event->accept();
  else
    event->ignore();
}

void QtRenderView::keyPressEvent(QKeyEvent *pEv)
{
  if (!pEv->isAutoRepeat())
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
    if (!spView || !spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
      pEv->ignore();
  }
}

void QtRenderView::keyReleaseEvent(QKeyEvent *pEv)
{
  if (!pEv->isAutoRepeat())
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
    if (!spView || !spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
      pEv->ignore();
  }
}

void QtRenderView::mouseDoubleClickEvent(QMouseEvent *pEv)
{
  // TODO: translate and process
  pEv->ignore();
}

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

  mouseLastX = x;
  mouseLastY = y;

  if (spView && spView->GetImpl<ep::ViewImpl>()->InputEvent(ev))
    pEv->accept();
  else
    pEv->ignore();
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
  else
    pEv->ignore();
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
  else
    pEv->ignore();
}

void QtRenderView::touchEvent(QTouchEvent *pEv)
{
  // TODO: translate and process
  pEv->ignore();
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

void QtRenderView::wheelEvent(QWheelEvent *pEv)
{
  // TODO: translate and process
  pEv->ignore();
}

} // namespace qt

#else
EPEMPTYFILE
#endif  // EPUI_DRIVER == EPDRIVER_QT
