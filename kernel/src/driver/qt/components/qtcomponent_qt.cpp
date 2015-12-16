#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "qtcomponent_qt.h"

namespace qt
{

QtComponent::QtComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  int64_t ptr = initParams.Get("object")->asInt();
  pQObject = (QObject*)(size_t)ptr;
}

QtComponent::~QtComponent()
{
}

} // namespace qt

#else
EPEMPTYFILE
#endif
