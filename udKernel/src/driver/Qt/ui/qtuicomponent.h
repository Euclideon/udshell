#pragma once
#ifndef QTUICOMPONENT_INTERNAL_H
#define QTUICOMPONENT_INTERNAL_H

#include <QQuickItem>

#include "components/uicomponent.h"

namespace qt
{

class ud::Kernel;

PROTOTYPE_COMPONENT(UIComponent);

class UIComponent : public ud::UIComponent
{
public:
  //UD_COMPONENT(UIComponent);

  QQuickItem *QuickItem() { return pQtQuickItem; }

protected:
  UIComponent(const ud::ComponentDesc *pType, ud::Kernel *pKernel, udRCString uid, InitParams initParams);
  virtual ~UIComponent();

  // HACK: allow ud::UIComponent::Create() to create a qt::UIComponent
  friend class ud::UIComponent;

private:
  QQuickItem *pQtQuickItem;
};

} // namespace qt

#endif  // QTUICOMPONENT_INTERNAL_H
