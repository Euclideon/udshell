#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "qtcomponent_qt.h"

namespace qt
{

ud::ComponentDesc QtComponent::descriptor =
{
  &ud::Component::descriptor, // pSuperDesc

  ud::UDSHELL_APIVERSION,         // udVersion
  ud::UDSHELL_PLUGINVERSION,      // pluginVersion

  "qtcomponent",              // id
  "qtcomponent",              // displayName
  "Is a Qt component",        // description
};

QtComponent::QtComponent(const ud::ComponentDesc *pType, ud::Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : ud::Component(pType, pKernel, uid, initParams)
{
  int64_t ptr = initParams["object"].asInt();
  pQObject = (QObject*)(size_t)ptr;
}

QtComponent::~QtComponent()
{
}

} // namespace qt

#endif
