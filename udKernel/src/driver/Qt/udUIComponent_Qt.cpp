#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "components/uicomponent.h"

namespace ud
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

} // namespace ud

#endif
