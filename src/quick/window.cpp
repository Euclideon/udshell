#include "udDebug.h"
#include "window.h"
#include "renderview.h"

Window::Window(QWindow *parent)
  : QQuickView(parent)
{
  // register new qml types
  qmlRegisterType<RenderView>("udShell", 0, 1, "RenderView");

  // modify our surface format to support opengl debug logging
  QSurfaceFormat format = QSurfaceFormat::defaultFormat();
  format.setOption(QSurfaceFormat::DebugContext);
  //format.setMajorVersion(4);
  //format.setMinorVersion(3);
  //format.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(format);

  // TODO - Use a shared context for udShell?
}

Window::~Window()
{
  udDebugPrintf("\nWindow::~Window()\n");
}
