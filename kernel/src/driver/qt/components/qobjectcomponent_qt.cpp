#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "qobjectcomponent_qt.h"

namespace qt {

QObjectComponent::QObjectComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  int64_t ptr = initParams.Get("object")->asInt();
  pQObject = (QObject*)(size_t)ptr;
}

QObjectComponent::~QObjectComponent()
{
}

} // namespace qt

#else
EPEMPTYFILE
#endif
