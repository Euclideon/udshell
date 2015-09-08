#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

// the qt implementation of the udui_driver requires a qt kernel currently
// error if we try otherwise
#if UDWINDOW_DRIVER != UDDRIVER_QT
#error UDUI_DRIVER Requires (UDWINDOW_DRIVER == UDDRIVER_QT)
#endif

#include <QQmlComponent>

#include "uicomponent.h"
#include "../udQtKernel_Internal.h"
#include "../util/typeconvert.h"


namespace qt
{

// ---------------------------------------------------------------------------------------
QtUIComponent::QtUIComponent(ud::ComponentDesc *pType, ud::Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : QtComponent(pType, pKernel, uid, initParams)
{
  QString filename = initParams["file"].as<QString>();
  if (filename.isNull())
  {
    udDebugPrintf("Error: attempted to create ui component without qml file set\n");
    udFree(pDesc);
    throw udR_Failure_;
  }

  // create the qml component for the associated script
  QtKernel *pQtKernel = static_cast<QtKernel*>(pKernel);
  QQmlComponent component(pQtKernel->QmlEngine(), QUrl(filename));
  pQtObject = component.create();

  if (!pQtObject)
  {
    // TODO: better error information/handling
    udDebugPrintf("Error creating QtUIComponent\n");
    foreach(const QQmlError &error, component.errors())
      udDebugPrintf("QML ERROR: %s\n", error.toString().toLatin1().data());
    udFree(pDesc);
    throw udR_Failure_;
  }

  // We expect a QQuickItem object
  // TODO: better error handling?
  UDASSERT(qobject_cast<QQuickItem*>(pQtObject) != nullptr, "QtUIComponents must create a QQuickItem");

  // Decorate the descriptor with meta object information
  PopulateComponentDesc(pQtObject);
}

// ---------------------------------------------------------------------------------------
QtUIComponent::~QtUIComponent()
{
}

// ---------------------------------------------------------------------------------------
QtViewport::QtViewport(ud::ComponentDesc *pType, ud::Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : QtComponent(pType, pKernel, uid, initParams)
{
  QString filename = initParams["file"].as<QString>();
  if (filename.isNull())
  {
    udDebugPrintf("Error: attempted to create ui component without qml file set\n");
    udFree(pDesc);
    throw udR_Failure_;
  }

  // create the qml component for the associated script
  QtKernel *pQtKernel = static_cast<QtKernel*>(pKernel);
  QQmlComponent component(pQtKernel->QmlEngine(), QUrl(filename));
  pQtObject = component.create();

  if (!pQtObject)
  {
    // TODO: better error information/handling
    udDebugPrintf("Error creating QtViewport\n");
    foreach(const QQmlError &error, component.errors())
      udDebugPrintf("QML ERROR: %s\n", error.toString().toLatin1().data());
    udFree(pDesc);
    throw udR_Failure_;
  }

  // We expect a QQuickItem object
  // TODO: better error handling?
  UDASSERT(qobject_cast<QQuickItem*>(pQtObject) != nullptr, "QtViewports must create a QQuickItem");

  // Decorate the descriptor with meta object information
  PopulateComponentDesc(pQtObject);
}

// ---------------------------------------------------------------------------------------
QtViewport::~QtViewport()
{
}

} // namespace qt

#endif  // UDUI_DRIVER == UDDRIVER_QT
