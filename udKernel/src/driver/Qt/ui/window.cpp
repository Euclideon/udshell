#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

// the qt implementation of the udui_driver requires a qt kernel currently
// error if we try otherwise
#if UDWINDOW_DRIVER != UDDRIVER_QT
#error UDUI_DRIVER Requires (UDWINDOW_DRIVER == UDDRIVER_QT)
#endif

#include <QQmlComponent>
#include <QLocale>

#include "window.h"
#include "../udQtKernel_Internal.h"
#include "../util/typeconvert.h"

namespace qt
{

// ---------------------------------------------------------------------------------------
QtWindow::QtWindow(ud::Component *pComponent, udString qml)
  : QtComponent(pComponent, qml)
{
  // We expect a QQuickWindow object
  // TODO: better error handling?
  UDASSERT(qobject_cast<QQuickWindow*>(pQtObject) != nullptr, "QtWindow must create a QQuickWindow");
}

// ---------------------------------------------------------------------------------------
QtWindow::~QtWindow()
{
}

// ---------------------------------------------------------------------------------------
void QtWindow::Refresh()
{

}

} // namespace qt

#endif // UDUI_DRIVER == UDDRIVER_QT
