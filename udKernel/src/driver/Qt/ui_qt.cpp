#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#if defined(_MSC_VER)
#pragma warning(disable:4512) // assignment operator could not be generated
#endif // defined(_MSC_VER)

#include <QQuickItem>
#include <QQuickWindow>

#include "udQtKernel_Internal.h"
#include "util/qmlbindings_qt.h"

#include "components/ui.h"
#include "components/viewport.h"
#include "components/window.h"

#include "ui/renderview.h"

namespace ud
{

udResult UIComponent::CreateInternal(udInitParams initParams)
{
  LogTrace("UIComponent::CreateInternal()");

  udString file = initParams["file"].as<udString>();
  if (file.empty())
  {
    LogError("Attempted to create ui component without source file");
    throw udR_Failure_;
  }

  // create the qml component for the associated script
  QString filename = QString::fromUtf8(file.ptr, static_cast<int>(file.length));
  QQmlComponent component(static_cast<qt::QtKernel*>(pKernel)->QmlEngine(), QUrl(filename));

  QObject *pQtObject = component.create();
  if (!pQtObject)
  {
    // TODO: better error information/handling
    LogError("Error creating QtComponent");
    foreach(const QQmlError &error, component.errors())
      LogError(udSharedString::concat("QML Error: ", error.toString().toLatin1().data()));
    throw udR_Failure_;
  }

  // We expect a QQuickItem object
  if (qobject_cast<QQuickItem*>(pQtObject) == nullptr)
  {
    LogError("UIComponent must create a QQuickItem");
    throw udR_Failure_;
  }

  // Decorate the descriptor with meta object information
  qt::PopulateComponentDesc<UIComponent>(this, pQtObject);

  pInternal = pQtObject;

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void UIComponent::DestroyInternal()
{
  LogTrace("UIComponent::DestroyInternal()");

  QQuickItem *pQtObject = (QQuickItem*)pInternal;
  delete pQtObject;
  pInternal = nullptr;
}


// ---------------------------------------------------------------------------------------
udResult Viewport::CreateInternal(udInitParams initParams)
{
  LogTrace("Viewport::CreateInternal()");

  // check that we have a RenderView
  QQuickItem *pRootItem = (QQuickItem*)pInternal;
  QList<qt::RenderView *> renderViews = pRootItem->findChildren<qt::RenderView *>();
  if (renderViews.size() != 1)
  {
    LogWarning(1, "Viewport component must contain 1 RenderView QML item");
    return udR_Failure_;
  }

  // check if we passed in a view, otherwise create a default one
  ViewRef spView = initParams["view"].as<ViewRef>();
  if (!spView)
  {
    LogDebug(2, "Creating internal view");
    spView = pKernel->CreateComponent<View>();
  }

  renderViews.first()->AttachView(spView);

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void Viewport::DestroyInternal()
{
  LogTrace("Viewport::DestroyInternal()");
}


// ---------------------------------------------------------------------------------------
udResult Window::CreateInternal(udInitParams initParams)
{
  LogTrace("Window::CreateInternal()");

  qt::QtKernel *pQtKernel = static_cast<qt::QtKernel*>(pKernel);

  udString file = initParams["file"].as<udString>();
  if (file.empty())
  {
    LogError("Attempted to create ui component without source file");
    throw udR_Failure_;
  }

  // create the qml component for the associated script
  QString filename = QString::fromUtf8(file.ptr, static_cast<int>(file.length));
  QQmlComponent component(pQtKernel->QmlEngine(), QUrl(filename));

  QObject *pQtObject = component.create();
  if (!pQtObject)
  {
    // TODO: better error information/handling
    LogError("Error creating QtComponent");
    foreach(const QQmlError &error, component.errors())
      LogError(udSharedString::concat("QML Error: ", error.toString().toLatin1().data()));
    throw udR_Failure_;
  }

  // We expect a QQuickWindow object
  QQuickWindow *pQtWindow = qobject_cast<QQuickWindow*>(pQtObject);
  if (pQtWindow == nullptr)
  {
    LogError("Window must create a QQuickWindow");
    throw udR_Failure_;
  }

  // Decorate the descriptor with meta object information
  qt::PopulateComponentDesc<Window>(this, pQtObject);

  pInternal = pQtObject;

  // register the window with the kernel
  if (pQtKernel->RegisterWindow(pQtWindow) != udR_Success)
  {
    // TODO: error handling
    LogError("Unable to register window");
    throw udR_Failure_;
  }

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void Window::DestroyInternal()
{
  LogTrace("Window::DestroyInternal()");

  QQuickWindow *pQtWindow = (QQuickWindow*)pInternal;
  delete pQtWindow;
  pInternal = nullptr;
}

// ---------------------------------------------------------------------------------------
void Window::SetTopLevelUI(UIComponentRef spUIComponent)
{
  LogTrace("Window::SetTopLevelUI()");

  spTopLevelUI = spUIComponent;
  QQuickWindow *pQtWindow = (QQuickWindow*)pInternal;

  // if there's an existing top level ui, then detach
  foreach(QQuickItem *pChild, pQtWindow->contentItem()->childItems())
    pChild->setParentItem(nullptr);

  // set the new one
  QQuickItem *pQtItem = reinterpret_cast<QQuickItem*>(spUIComponent->GetInternalData());
  if (pQtItem)
    pQtItem->setParentItem(pQtWindow->contentItem());
}

} // namespace ud

#endif
