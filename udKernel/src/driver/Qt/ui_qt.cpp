#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include <QQuickItem>
#include <QQuickWindow>

#include "udQtKernel_Internal.h"
#include "util/qmlbindings_qt.h"

#include "components/ui.h"
#include "components/viewport.h"
#include "components/window.h"

namespace ud
{

udResult UIComponent::CreateInternal(udInitParams initParams)
{
  // create the qml component for the associated script
  qt::QtKernel *pQtKernel = static_cast<qt::QtKernel*>(pKernel);

  udString file = initParams["file"].as<udString>();
  if (file.empty())
  {
    LogError("Attempted to create ui component without source file");
    throw udR_Failure_;
  }

  QString filename = QString::fromUtf8(file.ptr, static_cast<int>(file.length));
  QQmlComponent component(pQtKernel->QmlEngine(), QUrl(filename));

  QObject *pQtObject = component.create();
  if (!pQtObject)
  {
    // TODO: better error information/handling
    LogError("Error creating QtComponent");
    foreach(const QQmlError &error, component.errors())
      LogError(udSharedString::sprintf("QML ERROR: %s", error.toString().toLatin1().data()));
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

void UIComponent::DestroyInternal()
{
  QQuickItem *pQtObject = (QQuickItem*)pInternal;
  delete pQtObject;
  pInternal = nullptr;
}


udResult Viewport::CreateInternal(udInitParams initParams)
{
  return udR_Success;
}

void Viewport::DestroyInternal()
{
}


udResult Window::CreateInternal(udInitParams initParams)
{
  // create the qml component for the associated script
  qt::QtKernel *pQtKernel = static_cast<qt::QtKernel*>(pKernel);

  udString file = initParams["file"].as<udString>();
  if (file.empty())
  {
    LogError("Attempted to create ui component without source file");
    throw udR_Failure_;
  }

  QString filename = QString::fromUtf8(file.ptr, static_cast<int>(file.length));
  QQmlComponent component(pQtKernel->QmlEngine(), QUrl(filename));

  QObject *pQtObject = component.create();
  if (!pQtObject)
  {
    // TODO: better error information/handling
    LogError("Error creating QtComponent");
    foreach(const QQmlError &error, component.errors())
      LogError(udSharedString::sprintf("QML ERROR: %s", error.toString().toLatin1().data()));
    throw udR_Failure_;
  }

  // We expect a QQuickWindow object
  if (qobject_cast<QQuickWindow*>(pQtObject) == nullptr)
  {
    LogError("Window must create a QQuickWindow");
    throw udR_Failure_;
  }

  // Decorate the descriptor with meta object information
  qt::PopulateComponentDesc<Window>(this, pQtObject);

  pInternal = pQtObject;

  return udR_Success;
}

void Window::DestroyInternal()
{
  QQuickWindow *pQtObject = (QQuickWindow*)pInternal;
  delete pQtObject;
  pInternal = nullptr;
}

} // namespace ud

#endif
