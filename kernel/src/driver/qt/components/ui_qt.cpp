#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#if defined(_MSC_VER)
#pragma warning(disable:4512) // assignment operator could not be generated
#endif // defined(_MSC_VER)

#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>

#include "../epkernel_qt.h"
#include "../util/qmlbindings_qt.h"

#include "components/ui.h"
#include "components/viewport.h"
#include "components/window.h"

#include "../ui/renderview_qt.h"

namespace internal
{

// Helper function
udResult SetupFromQmlFile(epInitParams initParams, qt::QtKernel *pKernel, ep::Component *pComponent, QObject **ppInternal)
{
  epString file = initParams["file"].as<epString>();
  if (file.empty())
  {
    pComponent->LogError("Attempted to create ui component without source file");
    return udR_Failure_;
  }

  // create QObject wrapper for this component and expose it to the qml context for this ui component
  qt::QtEPComponent *pEPComponent = new qt::QtEPComponent(pComponent);
  QQmlContext *pContext = new QQmlContext(pKernel->QmlEngine()->rootContext());
  pContext->setContextProperty("thisComponent", pEPComponent);

  // create the qml component for the associated script
  QString filename = QString::fromUtf8(file.ptr, (int)file.length);
  QQmlComponent component(pKernel->QmlEngine(), QUrl(filename));

  QObject *pQtObject = component.create(pContext);
  if (!pQtObject)
  {
    // TODO: better error information/handling
    pComponent->LogError("Error creating QtComponent");
    foreach(const QQmlError &error, component.errors())
      pComponent->LogError(epSharedString::concat("QML Error: ", error.toString().toUtf8().data()));

    delete pContext;
    delete pEPComponent;

    return udR_Failure_;
  }

  // transfer ownership of our qt objects so they are cleaned up
  pContext->setParent(pQtObject);
  pEPComponent->setParent(pQtObject);

  *ppInternal = pQtObject;

  return udR_Success;
}

// Helper function
void CleanupInternalData(QObject **ppInternal)
{
  delete *ppInternal;
  *ppInternal = nullptr;
}

}   // namespace internal

namespace ep
{

using internal::SetupFromQmlFile;
using internal::CleanupInternalData;

// ---------------------------------------------------------------------------------------
udResult UIComponent::CreateInternal(epInitParams initParams)
{
  LogTrace("UIComponent::CreateInternal()");

  if (SetupFromQmlFile(initParams, (qt::QtKernel*)pKernel, this, (QObject**)&pInternal) != udR_Success)
    return udR_Failure_;

  QObject *pQtObject = (QObject*)pInternal;

  // We expect a QQuickItem object
  if (qobject_cast<QQuickItem*>(pQtObject) == nullptr)
  {
    LogError("UIComponent must create a QQuickItem");
    CleanupInternalData((QObject**)&pInternal);
    return udR_Failure_;
  }

  // Decorate the descriptor with meta object information
  qt::PopulateComponentDesc<UIComponent>(this, pQtObject);

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
udResult UIComponent::InitComplete()
{
  // let qml know that the enclosing object has finished being created
  QObject *pQtObject = (QObject*)pInternal;
  qt::QtEPComponent *epComponent = pQtObject->findChild<qt::QtEPComponent*>(QString(), Qt::FindDirectChildrenOnly);
  if (epComponent)
    epComponent->Done();

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void UIComponent::DestroyInternal()
{
  LogTrace("UIComponent::DestroyInternal()");
  CleanupInternalData((QObject**)&pInternal);
}


// ---------------------------------------------------------------------------------------
udResult Viewport::CreateInternal(epInitParams initParams)
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
udResult Window::CreateInternal(epInitParams initParams)
{
  LogTrace("Window::CreateInternal()");

  qt::QtKernel *pQtKernel = (qt::QtKernel*)pKernel;
  if (SetupFromQmlFile(initParams, pQtKernel, this, (QObject**)&pInternal) != udR_Success)
    return udR_Failure_;

  QQuickWindow *pQtWindow = qobject_cast<QQuickWindow*>((QObject*)pInternal);
  // We expect a QQuickWindow object
  if (pQtWindow == nullptr)
  {
    LogError("Window must create a QQuickWindow");
    CleanupInternalData((QObject**)&pInternal);
    return udR_Failure_;
  }

  // Decorate the descriptor with meta object information
  qt::PopulateComponentDesc<Window>(this, pQtWindow);

  // register the window with the kernel
  if (pQtKernel->RegisterWindow(pQtWindow) != udR_Success)
  {
    // TODO: error handling
    LogError("Unable to register window");
    CleanupInternalData((QObject**)&pInternal);
    return udR_Failure_;
  }

  return udR_Success;
}

// ---------------------------------------------------------------------------------------
void Window::DestroyInternal()
{
  LogTrace("Window::DestroyInternal()");
  CleanupInternalData((QObject**)&pInternal);
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
  QQuickItem *pQtItem = (QQuickItem*)spUIComponent->GetInternalData();
  if (pQtItem)
    pQtItem->setParentItem(pQtWindow->contentItem());
}

} // namespace ep

#endif
