#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <QQuickWindow>

#include "qtcomponent.h"
#include "components/ui.h"

namespace ud {
  class Kernel;
}

namespace qt
{

PROTOTYPE_COMPONENT(QtWindow);

class QtWindow : public QtComponent
{
public:
  QtWindow(ud::Component *pComponent, udString qml);
  virtual ~QtWindow();

  QQuickWindow *QuickWindow() { return static_cast<QQuickWindow*>(pQtObject); }

  void Refresh();
};

} // namespace qt

#endif  // WINDOW_H
