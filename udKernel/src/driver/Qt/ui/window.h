#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <QQuickWindow>

#include "qtcomponent.h"
#include "components/ui.h"

namespace qt
{

class ud::Kernel;

PROTOTYPE_COMPONENT(QtWindow);

class QtWindow : public QtComponent < ud::Window >
{
public:
  QQuickWindow *QuickWindow() { return static_cast<QQuickWindow*>(pQtObject); }

protected:
  QtWindow(ud::ComponentDesc *pType, ud::Kernel *pKernel, udRCString uid, udInitParams initParams);
  virtual ~QtWindow();

  void Refresh();

  // HACK: allow ud::Window::Create() to create a QtWindow
  friend class ud::Window;
};

} // namespace qt

#endif  // WINDOW_H
