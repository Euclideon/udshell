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
#include "driver/qt/util/qmlbindings_qt.h"

ep::KeyCode qtKeyToEPKey(Qt::Key qk);

namespace qt {

class QtFboRenderer : public QQuickFramebufferObject::Renderer
{
public:
  QtFboRenderer(const QQuickFramebufferObject *item) : m_item(item) { }

  void render()
  {
    if (spRenderableView)
      spRenderableView->RenderGPU();

    // Qt doesn't reset the cull mode
    epRenderState_SetCullMode(false, epF_CCW);

    if (m_item->window())
        m_item->window()->resetOpenGLState();
  }

  QOpenGLFramebufferObject *createFramebufferObject(const QSize &size)
  {
    // TODO: Set up appropriate format
    QOpenGLFramebufferObjectFormat format;
    //format.setInternalTextureFormat(GL_RGBA8);
    format.setAttachment(QOpenGLFramebufferObject::Depth);
    return new QOpenGLFramebufferObject(size, format);
  }

  void synchronize(QQuickFramebufferObject * item)
  {
    QtRenderView *pQtRenderView = (QtRenderView*)item;

    if (pQtRenderView->dirty)
    {
      spRenderableView = ep::SynchronisedPtr<ep::RenderableView>(pQtRenderView->spView->getImpl<ep::ViewImpl>()->getRenderableView(), QtApplication::kernel());
      pQtRenderView->dirty = false;
    }
  }

  const QQuickFramebufferObject *m_item;
  ep::SynchronisedPtr<ep::RenderableView> spRenderableView;
};

ep::MouseControls GetMouseButton(Qt::MouseButton button)
{
  // TODO: qt mouse buttons is a bit field
  if (button & Qt::LeftButton)   return ep::MouseControls::LeftButton;
  if (button & Qt::MiddleButton) return ep::MouseControls::MiddleButton;
  if (button & Qt::RightButton)  return ep::MouseControls::RightButton;
  if (button & Qt::ExtraButton1) return ep::MouseControls::Button4;
  if (button & Qt::ExtraButton2) return ep::MouseControls::Button5;

  return ep::MouseControls::Max; // redundant return for not yet supported buttons.
}

QtRenderView::QtRenderView(QQuickItem *pParent)
  : QQuickFramebufferObject(pParent)
{
  QObject::connect(this, &QQuickItem::widthChanged, this, &QtRenderView::onResize);
  QObject::connect(this, &QQuickItem::heightChanged, this, &QtRenderView::onResize);
  QObject::connect(this, &QQuickItem::visibleChanged, this, &QtRenderView::onVisibleChanged);

  setMirrorVertically(true);
}

QtRenderView::~QtRenderView()
{
  if (spView)
  {
    spView->frameReady.unsubscribe(ep::Delegate<void()>(this, &QtRenderView::onFrameReady));
    spView->getImpl<ep::ViewImpl>()->setLatestFrame(nullptr);
  }
}

QQuickFramebufferObject::Renderer *QtRenderView::createRenderer() const
{
  return new QtFboRenderer(this);
}

void QtRenderView::setView(const QVariant &view)
{
  using namespace ep;
  ViewRef newView = component_cast<View>(Variant(view).asComponent());

  if (spView != newView)
  {
    if (spView)
    {
      spView->frameReady.unsubscribe(ep::Delegate<void()>(this, &QtRenderView::onFrameReady));
      spView->getImpl<ep::ViewImpl>()->setLatestFrame(nullptr);
    }

    spView = newView;

    if (spView)
    {
      spView->logDebug(2, "Attaching View Component '{0}' to Render Viewport", spView->getUid());
      spView->frameReady.subscribe(Delegate<void()>(this, &QtRenderView::onFrameReady));
    }

    // TEMP HAX:
    QtApplication::kernel()->setFocusView(spView);
  }
}

void QtRenderView::onResize()
{
  int w = (int)width();
  int h = (int)height();

  if (spView && w > 0 && h > 0)
    spView->resize(w, h);
}

void QtRenderView::onVisibleChanged()
{
  if (spView)
  {
    if (isVisible())
      spView->activate();
    else
      spView->deactivate();
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

void QtRenderView::focusInEvent(QFocusEvent * event)
{
  ep::InputEvent ev;
  ev.deviceType = ep::InputDevice::Keyboard;
  ev.deviceId = 0;
  ev.eventType = ep::InputEvent::EventType::Focus;
  ev.hasFocus = true;

  if (spView && spView->getImpl<ep::ViewImpl>()->inputEvent(ev))
    event->accept();
  else
    event->ignore();
}

void QtRenderView::focusOutEvent(QFocusEvent * event)
{
  ep::InputEvent ev;
  ev.deviceType = ep::InputDevice::Keyboard;
  ev.deviceId = 0;
  ev.eventType = ep::InputEvent::EventType::Focus;
  ev.hasFocus = false;

  if (spView && spView->getImpl<ep::ViewImpl>()->inputEvent(ev))
    event->accept();
  else
    event->ignore();
}

void QtRenderView::keyPressEvent(QKeyEvent *pEv)
{
  if (!pEv->isAutoRepeat())
  {
    ep::KeyCode kc = qtKeyToEPKey((Qt::Key)pEv->key());
    if (kc == ep::KeyCode::Unknown)
      return;
    ep::InputEvent ev;
    ev.deviceType = ep::InputDevice::Keyboard;
    ev.deviceId = 0; // TODO: get keyboard id
    ev.eventType = ep::InputEvent::EventType::Key;
    ev.key.key = (int)kc;
    ev.key.state = 1;
    if (!spView || !spView->getImpl<ep::ViewImpl>()->inputEvent(ev))
      pEv->ignore();
  }
}

void QtRenderView::keyReleaseEvent(QKeyEvent *pEv)
{
  if (!pEv->isAutoRepeat())
  {
    ep::KeyCode kc = qtKeyToEPKey((Qt::Key)pEv->key());
    if (kc == ep::KeyCode::Unknown)
      return;
    ep::InputEvent ev;
    ev.deviceType = ep::InputDevice::Keyboard;
    ev.deviceId = 0; // TODO: get keyboard id
    ev.eventType = ep::InputEvent::EventType::Key;
    ev.key.key = (int)kc;
    ev.key.state = 0;
    if (!spView || !spView->getImpl<ep::ViewImpl>()->inputEvent(ev))
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

  ep::InputEvent ev;
  ev.deviceType = ep::InputDevice::Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = ep::InputEvent::EventType::Move;
  ev.move.xDelta = (float)(mouseLastX - x);
  ev.move.yDelta = (float)(mouseLastY - y);
  ev.move.xAbsolute = (float)x;
  ev.move.yAbsolute = (float)y;

  mouseLastX = x;
  mouseLastY = y;

  if (spView && spView->getImpl<ep::ViewImpl>()->inputEvent(ev))
    pEv->accept();
  else
    pEv->ignore();
}

void QtRenderView::mousePressEvent(QMouseEvent *pEv)
{
  if ((pEv->buttons() & Qt::LeftButton) && !hasActiveFocus())
    forceActiveFocus();

  ep::InputEvent ev;
  ev.deviceType = ep::InputDevice::Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = ep::InputEvent::EventType::Key;
  ev.key.key = GetMouseButton(pEv->button());
  ev.key.state = 1;
  if (ev.key.key != ep::MouseControls::Max && spView && spView->getImpl<ep::ViewImpl>()->inputEvent(ev))
    pEv->accept();
  else
    pEv->ignore();
}

void QtRenderView::mouseReleaseEvent(QMouseEvent *pEv)
{
  ep::InputEvent ev;
  ev.deviceType = ep::InputDevice::Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = ep::InputEvent::EventType::Key;
  ev.key.key = GetMouseButton(pEv->button());
  ev.key.state = 0;
  if (ev.key.key != ep::MouseControls::Max && spView && spView->getImpl<ep::ViewImpl>()->inputEvent(ev))
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

  ep::InputEvent ev;
  ev.deviceType = ep::InputDevice::Mouse;
  ev.deviceId = 0; // TODO: get mouse id
  ev.eventType = ep::InputEvent::EventType::Move;
  ev.move.xDelta = (float)(mouseLastX - x);
  ev.move.yDelta = (float)(mouseLastY - y);
  ev.move.xAbsolute = (float)x;
  ev.move.yAbsolute = (float)y;

  if (spView)
    spView->getImpl<ep::ViewImpl>()->inputEvent(ev);
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
