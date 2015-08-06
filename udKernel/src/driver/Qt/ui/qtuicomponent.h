#pragma once
#ifndef QTUICOMPONENT_INTERNAL_H
#define QTUICOMPONENT_INTERNAL_H

#include <QQuickItem>

#include "components/uicomponent.h"

namespace qt
{

class ud::Kernel;

PROTOTYPE_COMPONENT(QtUIComponent);

template<typename Base>
class QtComponent : public Base
{
public:
  virtual const ud::PropertyInfo *GetPropertyInfo(int index) const;
  virtual const ud::PropertyInfo *GetPropertyInfo(udString property) const;

  virtual size_t NumProperties() const;
  //virtual ptrdiff_t PropertyIndex(udString property) const;

  virtual void SetProperty(udString property, const udVariant &value);
  virtual udVariant GetProperty(udString property) const;

protected:
  QtComponent(const ud::ComponentDesc *pType, ud::Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Base(pType, pKernel, uid, initParams), pQtObject(nullptr) {}
  virtual ~QtComponent() {}

  virtual void Init(udInitParams initParams);

  //static ud::ComponentDesc *CreateComponentDesc(const ud::ComponentDesc *pType);

  //void DecorateComponentDesc(QObject *pObject);

protected:
  QObject *pQtObject;
  //ud::ComponentDesc *pDesc;
};


class QtUIComponent : public QtComponent < ud::UIComponent >
{
public:
  QQuickItem *QuickItem() { return pQtQuickItem; }

protected:
  QtUIComponent(const ud::ComponentDesc *pType, ud::Kernel *pKernel, udRCString uid, udInitParams initParams);
  virtual ~QtUIComponent();

  // HACK: allow ud::UIComponent::Create() to create a qt::UIComponent
  friend class ud::UIComponent;

  QQuickItem *pQtQuickItem;
};


// ---------------------------------------------------------------------------------------
/*template<typename Base>
ud::ComponentDesc *QtComponent<Base>::CreateComponentDesc(const ud::ComponentDesc *pType)
{
  ud::ComponentDesc *pCompDesc = udAllocType(ud::ComponentDesc, 1, udAF_Zero);

  // TODO: make an internal component lookup table if we end up needing multiple components for the one qml file

  pCompDesc->pSuperDesc = &ud::UIComponent::descriptor;
  pCompDesc->udVersion = ud::UDSHELL_APIVERSION;
  pCompDesc->pluginVersion = ud::UDSHELL_PLUGINVERSION;

  // TODO: should we use unique qt id's here?
  pCompDesc->id = ud::UIComponent::descriptor.id;
  pCompDesc->displayName = ud::UIComponent::descriptor.displayName;
  pCompDesc->description = ud::UIComponent::descriptor.description;

  return pCompDesc;
}*/

} // namespace qt

#endif  // QTUICOMPONENT_INTERNAL_H
