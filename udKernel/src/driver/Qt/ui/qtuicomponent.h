#pragma once
#ifndef QTUICOMPONENT_H
#define QTUICOMPONENT_H

#include <QQuickItem>

#include "components/uicomponent.h"

namespace qt
{

class ud::Kernel;

PROTOTYPE_COMPONENT(QtUIComponent);

// Decorator base class
template<typename Base>
class QtComponent : public Base
{
public:
  QObject *QtObject() { return pQtObject; }
  const QObject *QtObject() const { return pQtObject; }

protected:
  QtComponent(ud::ComponentDesc *pType, ud::Kernel *pKernel, udSharedString uid, udInitParams initParams)
    : Base(pType, pKernel, uid, initParams), pDesc(pType), pQtObject(nullptr) {}
  virtual ~QtComponent();

  static ud::ComponentDesc *CreateComponentDesc(const ud::ComponentDesc *pType);
  void PopulateComponentDesc(QObject *pObject);

  ud::ComponentDesc *pDesc;
  QObject *pQtObject;
};


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

} // namespace qt

#endif  // QTUICOMPONENT_H
