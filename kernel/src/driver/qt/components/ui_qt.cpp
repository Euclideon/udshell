#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

// Disabled Warnings
#if defined(EP_COMPILER_VISUALC)
# pragma warning(disable:4512) // assignment operator could not be generated
#endif //defined(_MSC_VER)

#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>

#include "../epkernel_qt.h"
#include "../util/qmlbindings_qt.h"
#include "component_qt.h"
#include "qtcomponent_qt.h"

#include "components/ui.h"
#include "components/viewport.h"
#include "components/window.h"

#include "../ui/renderview_qt.h"

namespace qt {
namespace internal {

// Helper function
epResult SetupFromQmlFile(Variant::VarMap initParams, qt::QtKernel *pKernel, Component *pComponent, QObject **ppInternal)
{
  String file = initParams.Get("file")->as<String>();
  if (file.empty())
  {
    pComponent->LogError("Attempted to create ui component without source file");
    return epR_Failure;
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
      pComponent->LogError(SharedString::concat("QML Error: ", error.toString().toUtf8().data()));

    delete pContext;
    delete pEPComponent;

    return epR_Failure;
  }

  // transfer ownership of our qt objects so they are cleaned up
  pContext->setParent(pQtObject);
  pEPComponent->setParent(pQtObject);

  *ppInternal = pQtObject;

  return epR_Success;
}

// Helper function
void CleanupInternalData(QObject **ppInternal)
{
  delete *ppInternal;
  *ppInternal = nullptr;
}

} // namespace internal
} // namespace qt


namespace kernel {

using qt::internal::SetupFromQmlFile;
using qt::internal::CleanupInternalData;

// ---------------------------------------------------------------------------------------
Variant UIComponent::GetUIHandle() const
{
  return GetKernel().CreateComponent<qt::QtComponent>({ { "object", (int64_t)(size_t)pUserData } });
}

// ---------------------------------------------------------------------------------------
epResult UIComponent::CreateInternal(Variant::VarMap initParams)
{
  LogTrace("UIComponent::CreateInternal()");

  if (SetupFromQmlFile(initParams, (qt::QtKernel*)pKernel, this, (QObject**)&pUserData) != epR_Success)
    return epR_Failure;

  QObject *pQtObject = (QObject*)pUserData;

  // We expect a QQuickItem object
  if (qobject_cast<QQuickItem*>(pQtObject) == nullptr)
  {
    LogError("UIComponent must create a QQuickItem");
    CleanupInternalData((QObject**)&pUserData);
    return epR_Failure;
  }

  // Decorate the descriptor with meta object information
  qt::PopulateComponentDesc(this, pQtObject);

  return epR_Success;
}

// ---------------------------------------------------------------------------------------
epResult UIComponent::InitComplete()
{
  // let qml know that the enclosing object has finished being created
  QObject *pQtObject = (QObject*)pUserData;
  qt::QtEPComponent *epComponent = pQtObject->findChild<qt::QtEPComponent*>(QString(), Qt::FindDirectChildrenOnly);
  if (epComponent)
    epComponent->Done();

  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void UIComponent::DestroyInternal()
{
  LogTrace("UIComponent::DestroyInternal()");
  CleanupInternalData((QObject**)&pUserData);
}


// ---------------------------------------------------------------------------------------
epResult Viewport::CreateInternal(Variant::VarMap initParams)
{
  LogTrace("Viewport::CreateInternal()");

  // check that we have a RenderView
  QQuickItem *pRootItem = (QQuickItem*)pUserData;
  QList<qt::RenderView *> renderViews = pRootItem->findChildren<qt::RenderView *>();
  if (renderViews.size() != 1)
  {
    LogWarning(1, "Viewport component must contain 1 RenderView QML item");
    return epR_Failure;
  }

  // check if we passed in a view, otherwise create a default one
  ViewRef spView = initParams.Get("view")->as<ViewRef>();
  if (!spView)
  {
    LogDebug(2, "Creating internal view");
    spView = GetKernel().CreateComponent<View>();
  }

  renderViews.first()->AttachView(spView);

  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void Viewport::DestroyInternal()
{
  LogTrace("Viewport::DestroyInternal()");
}


// ---------------------------------------------------------------------------------------
epResult Window::CreateInternal(Variant::VarMap initParams)
{
  LogTrace("Window::CreateInternal()");

  qt::QtKernel *pQtKernel = (qt::QtKernel*)pKernel;
  if (SetupFromQmlFile(initParams, pQtKernel, this, (QObject**)&pUserData) != epR_Success)
    return epR_Failure;

  QQuickWindow *pQtWindow = qobject_cast<QQuickWindow*>((QObject*)pUserData);
  // We expect a QQuickWindow object
  if (pQtWindow == nullptr)
  {
    LogError("Window must create a QQuickWindow");
    CleanupInternalData((QObject**)&pUserData);
    return epR_Failure;
  }

  // Decorate the descriptor with meta object information
  qt::PopulateComponentDesc(this, pQtWindow);

  // register the window with the kernel
  if (pQtKernel->RegisterWindow(pQtWindow) != epR_Success)
  {
    // TODO: error handling
    LogError("Unable to register window");
    CleanupInternalData((QObject**)&pUserData);
    return epR_Failure;
  }

  return epR_Success;
}

// ---------------------------------------------------------------------------------------
void Window::DestroyInternal()
{
  LogTrace("Window::DestroyInternal()");
  CleanupInternalData((QObject**)&pUserData);
}

// ---------------------------------------------------------------------------------------
void Window::SetTopLevelUI(UIComponentRef spUIComponent)
{
  LogTrace("Window::SetTopLevelUI()");

  spTopLevelUI = spUIComponent;
  QQuickWindow *pQtWindow = (QQuickWindow*)pUserData;

  // if there's an existing top level ui, then detach
  foreach(QQuickItem *pChild, pQtWindow->contentItem()->childItems())
    pChild->setParentItem(nullptr);

  // set the new one
  QQuickItem *pQtItem = (QQuickItem*)spUIComponent->GetUserData();
  if (pQtItem)
    pQtItem->setParentItem(pQtWindow->contentItem());
}

} // namespace kernel

#else
EPEMPTYFILE
#endif
