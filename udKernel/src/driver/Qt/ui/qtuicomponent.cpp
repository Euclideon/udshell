#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

// the qt implementation of the udui_driver requires a qt kernel currently
// error if we try otherwise
#if UDWINDOW_DRIVER != UDDRIVER_QT
#error UDUI_DRIVER Requires (UDWINDOW_DRIVER == UDDRIVER_QT)
#endif

#include <QQmlComponent>

#include "../udQtKernel_Internal.h"
#include "qtuicomponent.h"

namespace qt
{

// ---------------------------------------------------------------------------------------
UIComponent::UIComponent(const ud::ComponentDesc *pType, ud::Kernel *pKernel, udRCString uid, udInitParams initParams)
  : ud::UIComponent(pType, pKernel, uid, initParams)
{
  // create the qml component for the associated script
  // TODO: remove hardcoded script
  QtKernel *pQtKernel = static_cast<QtKernel*>(pKernel);
  QQmlComponent component(pQtKernel->QmlEngine(), QUrl(QStringLiteral("qrc:/qml/main.qml")));
  QObject *pObject = component.create();

  if (!pObject)
  {
    // TODO: better error information/handling
    udDebugPrintf("Error creating QtUIComponent\n");
    foreach(const QQmlError &error, component.errors())
      udDebugPrintf("QML ERROR: %s\n", error.toString().toLatin1().data());
    throw udR_Failure_;
  }

  // Try to cast to a QQuickItem
  // TODO: better error handling?
  pQtQuickItem = qobject_cast<QQuickItem*>(pObject);
  if (!pQtQuickItem)
    throw udR_Failure_;
}

// ---------------------------------------------------------------------------------------
UIComponent::~UIComponent()
{
  delete pQtQuickItem;
}

} // namespace qt

#endif  // UDUI_DRIVER == UDDRIVER_QT
