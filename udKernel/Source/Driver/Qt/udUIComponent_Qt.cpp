#include "udDriver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "udUIComponent.h"

namespace udKernel
{

class QmlComponent : public UIComponent
{
public:
  //UD_COMPONENT(udUiComponent);

  virtual void SetSource(udString source) {}

  // qml properties
  // qquickitem?
  // qqmlcomponent

protected:
  QmlComponent(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : UIComponent(pType, pKernel, uid, initParams) {
  }

  // HACK: expose udQmlComponent::Create to udUIComponent::descriptor
  friend class UIComponent;
};


Component *UIComponent::Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
{
  return udNew(QmlComponent, pType, pKernel, uid, initParams);
}

} // namespace udKernel

#endif
