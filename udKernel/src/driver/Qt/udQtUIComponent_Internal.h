#pragma once
#ifndef UDQTUICOMPONENT_INTERNAL_H
#define UDQTUICOMPONENT_INTERNAL_H

#include <QQuickItem>

#include "components/uicomponent.h"

namespace ud
{

class Kernel;

PROTOTYPE_COMPONENT(QtUIComponent);

class QtUIComponent : public UIComponent
{
public:
  //UD_COMPONENT(udUiComponent);

  QQuickItem *QuickItem() { return pQtQuickItem; }

  //virtual void SetSource(udString source) {}

  // qml properties
  // qquickitem?
  // qqmlcomponent

protected:
  QtUIComponent(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams);
  virtual ~QtUIComponent();

  // HACK: expose QtUIComponent::Create to UIComponent::descriptor
  friend class UIComponent;

private:
  QQuickItem *pQtQuickItem;
};

} // namespace ud

#endif  // UDQTUICOMPONENT_INTERNAL_H
