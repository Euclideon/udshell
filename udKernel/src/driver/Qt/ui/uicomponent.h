#pragma once
#ifndef UICOMPONENT_H
#define UICOMPONENT_H

#include <QQuickItem>

#include "qtcomponent.h"
#include "components/ui.h"

namespace qt
{

class ud::Kernel;

PROTOTYPE_COMPONENT(QtUIComponent);
PROTOTYPE_COMPONENT(QtViewport);

class QtUIComponent : public QtComponent < ud::UIComponent >
{
public:
  QQuickItem *QuickItem() { return static_cast<QQuickItem*>(pQtObject); }

protected:
  QtUIComponent(ud::ComponentDesc *pType, ud::Kernel *pKernel, udSharedString uid, udInitParams initParams);
  virtual ~QtUIComponent();

  // HACK: allow ud::UIComponent::Create() to create a QtUIComponent
  friend class ud::UIComponent;
};

class QtViewport : public QtComponent < ud::Viewport >
{
public:
  QQuickItem *QuickItem() { return static_cast<QQuickItem*>(pQtObject); }

protected:
  QtViewport(ud::ComponentDesc *pType, ud::Kernel *pKernel, udSharedString uid, udInitParams initParams);
  virtual ~QtViewport();

  // HACK: allow ud::Viewport::Create() to create a QtViewport
  friend class ud::Viewport;
};

} // namespace qt

#endif  // UICOMPONENT_H
