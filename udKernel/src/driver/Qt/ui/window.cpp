#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "window.h"
#include "renderview.h"

namespace qt
{

Window::Window(QWindow *parent)
  : QQuickWindow(parent)
{
  udDebugPrintf("Window::Window()\n");

  // register our internal qml types
  qmlRegisterType<RenderView>("udKernel", 0, 1, "RenderView");

  // modify our surface format to support opengl debug logging
  // TODO: set gl version based on property settings?
  QSurfaceFormat format = QSurfaceFormat::defaultFormat();
  format.setOption(QSurfaceFormat::DebugContext);
  setFormat(format);

  // TODO - Use a shared context for udShell?
}

Window::~Window()
{
  udDebugPrintf("Window::~Window()\n");
}

} // namespace qt

#endif // UDUI_DRIVER == UDDRIVER_QT
