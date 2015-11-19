#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "qtcomponent_qt.h"

namespace qt
{

kernel::ComponentDesc QtComponent::descriptor =
{
  &kernel::Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION,         // epVersion
  EPSHELL_PLUGINVERSION,      // pluginVersion

  "qtcomponent",              // id
  "qtcomponent",              // displayName
  "Is a Qt component",        // description
};

QtComponent::QtComponent(const kernel::ComponentDesc *pType, kernel::Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  int64_t ptr = initParams["object"].asInt();
  pQObject = (QObject*)(size_t)ptr;
}

QtComponent::~QtComponent()
{
}

} // namespace qt

#else
EPEMPTYFILE
#endif
