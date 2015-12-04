#pragma once
#ifndef EP_QTCOMPONENT_QT_H
#define EP_QTCOMPONENT_QT_H

#include <QObject>

#include "components/component.h"

namespace kernel {
class Kernel;
}

namespace qt
{

PROTOTYPE_COMPONENT(QtComponent);

class QtComponent : public kernel::Component
{
  EP_DECLARE_COMPONENT(QtComponent, kernel::Component, EPKERNEL_PLUGINVERSION, "QtComponent desc...")
public:

  QtComponent(const kernel::ComponentDesc *pType, kernel::Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~QtComponent();

  QObject *GetQObject() const { return pQObject; }

private:
  QObject *pQObject;
};

} // namespace qt

#endif // EP_QTCOMPONENT_QT_H
