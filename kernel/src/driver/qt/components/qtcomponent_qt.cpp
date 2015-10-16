#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "qtcomponent_qt.h"

namespace qt
{

ComponentDesc QtComponent::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION,         // epVersion
  EPSHELL_PLUGINVERSION,      // pluginVersion

  "qtcomponent",              // id
  "qtcomponent",              // displayName
  "Is a Qt component",        // description
};

QtComponent::QtComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
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
