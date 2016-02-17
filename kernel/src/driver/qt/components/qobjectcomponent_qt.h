#pragma once
#ifndef EP_QOBJECTCOMPONENT_QT_H
#define EP_QOBJECTCOMPONENT_QT_H

#include "ep/cpp/component/component.h"

#include "driver/qt/epqt.h"

namespace ep {
class Kernel;
}

class QObject;

namespace qt
{

SHARED_CLASS(QObjectComponent);

// This shim class wraps a QObject in an ep::Component that is accessible from the EP component system
class QObjectComponent : public Component
{
  EP_DECLARE_COMPONENT(QObjectComponent, Component, EPKERNEL_PLUGINVERSION, "QObjectComponent desc...")
public:

  QObjectComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~QObjectComponent();

  QObject *GetQObject() const { return pQObject; }

private:
  QObject *pQObject;
};

} // namespace qt

#endif // EP_QOBJECTCOMPONENT_QT_H
