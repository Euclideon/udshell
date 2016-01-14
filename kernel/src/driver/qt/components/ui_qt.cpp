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

#include "components/uicomponentimpl.h"
#include "components/viewportimpl.h"
#include "components/windowimpl.h"

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


namespace ep {

using qt::internal::SetupFromQmlFile;
using qt::internal::CleanupInternalData;

// ---------------------------------------------------------------------------------------
Variant UIComponentImpl::GetUIHandle() const
{
  return GetKernel()->CreateComponent<qt::QtComponent>({ { "object", (int64_t)(size_t)pInstance->pUserData } });
}

// ---------------------------------------------------------------------------------------
void UIComponentImpl::CreateInternal(Variant::VarMap initParams)
{
  LogTrace("UIComponentImpl::CreateInternal()");

  if (SetupFromQmlFile(initParams, (qt::QtKernel*)pInstance->pKernel, pInstance, (QObject**)&pInstance->pUserData) != epR_Success)
    EPTHROW_ERROR(epR_Failure, "Error creating QML based UIComponent");

  epscope(fail) { DestroyInternal(); };

  // We expect a QQuickItem object
  QObject *pQtObject = (QObject*)pInstance->pUserData;
  EPTHROW_IF(qobject_cast<QQuickItem*>(pQtObject) == nullptr, epR_Failure, "UIComponent must create a QQuickItem");

  // Decorate the descriptor with meta object information
  qt::PopulateComponentDesc(pInstance, pQtObject);
}

// ---------------------------------------------------------------------------------------
void UIComponentImpl::InitComplete()
{
  // let qml know that the enclosing object has finished being created
  QObject *pQtObject = (QObject*)pInstance->pUserData;
  qt::QtEPComponent *epComponent = pQtObject->findChild<qt::QtEPComponent*>(QString(), Qt::FindDirectChildrenOnly);
  if (epComponent)
    epComponent->Done();
}

// ---------------------------------------------------------------------------------------
void UIComponentImpl::DestroyInternal()
{
  LogTrace("UIComponentImpl::DestroyInternal()");
  CleanupInternalData((QObject**)&pInstance->pUserData);
}


// ---------------------------------------------------------------------------------------
void ViewportImpl::CreateInternal(Variant::VarMap initParams)
{
  LogTrace("Viewport::CreateInternal()");

  // check that we have a RenderView
  QQuickItem *pRootItem = (QQuickItem*)pInstance->pUserData;
  QList<qt::RenderView *> renderViews = pRootItem->findChildren<qt::RenderView *>();
  EPTHROW_IF(renderViews.size() != 1, epR_Failure, "Viewport component must contain only one RenderView QML item");

  // check if we passed in a view, otherwise create a default one
  spView = initParams.Get("view")->as<ViewRef>();
  if (!spView)
  {
    LogDebug(2, "Creating internal View component");
    spView = GetKernel()->CreateComponent<View>();
  }

  LogDebug(2, "Attaching View Component '{0}' to Viewport", spView->GetUid());
  renderViews.first()->AttachView(spView);
}


// ---------------------------------------------------------------------------------------
void WindowImpl::CreateInternal(Variant::VarMap initParams)
{
  LogTrace("WindowImpl::CreateInternal()");

  qt::QtKernel *pQtKernel = (qt::QtKernel*)pInstance->pKernel;
  if (SetupFromQmlFile(initParams, pQtKernel, pInstance, (QObject**)&pInstance->pUserData) != epR_Success)
    EPTHROW_ERROR(epR_Failure, "Error creating QML based Window Component");

  epscope(fail) { DestroyInternal(); };

  // we expect a QQuickWindow object
  QQuickWindow *pQtWindow = qobject_cast<QQuickWindow*>((QObject*)pInstance->pUserData);
  EPTHROW_IF(pQtWindow == nullptr, epR_Failure, "Window component must create a QQuickWindow");

  // decorate the descriptor with meta object information
  qt::PopulateComponentDesc(pInstance, pQtWindow);

  // register the window with the kernel
  if (pQtKernel->RegisterWindow(pQtWindow) != epR_Success)
    EPTHROW_ERROR(epR_Failure, "Unable to register Window component with Kernel");
}

// ---------------------------------------------------------------------------------------
void WindowImpl::DestroyInternal()
{
  LogTrace("WindowImpl::DestroyInternal()");
  CleanupInternalData((QObject**)&pInstance->pUserData);
}

// ---------------------------------------------------------------------------------------
void WindowImpl::SetTopLevelUI(UIComponentRef spUIComponent)
{
  LogTrace("WindowImpl::SetTopLevelUI()");

  spTopLevelUI = spUIComponent;
  QQuickWindow *pQtWindow = (QQuickWindow*)pInstance->pUserData;

  // if there's an existing top level ui, then detach
  foreach(QQuickItem *pChild, pQtWindow->contentItem()->childItems())
    pChild->setParentItem(nullptr);

  // set the new one
  QQuickItem *pQtItem = (QQuickItem*)spUIComponent->GetUserData();
  if (pQtItem)
    pQtItem->setParentItem(pQtWindow->contentItem());
}

} // namespace ep

#else
EPEMPTYFILE
#endif
