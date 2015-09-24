#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "qtcomponent_qt.h"

namespace qt
{

ep::ComponentDesc QtComponent::descriptor =
{
  &ep::Component::descriptor, // pSuperDesc

  ep::UDSHELL_APIVERSION,         // udVersion
  ep::UDSHELL_PLUGINVERSION,      // pluginVersion

  "qtcomponent",              // id
  "qtcomponent",              // displayName
  "Is a Qt component",        // description
};

QtComponent::QtComponent(const ep::ComponentDesc *pType, ep::Kernel *pKernel, epSharedString uid, epInitParams initParams)
  : ep::Component(pType, pKernel, uid, initParams)
{
  int64_t ptr = initParams["object"].asInt();
  pQObject = (QObject*)(size_t)ptr;
}

QtComponent::~QtComponent()
{
}

} // namespace qt

#endif
