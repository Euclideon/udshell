#pragma once
#ifndef EP_QTCOMPONENT_QT_H
#define EP_QTCOMPONENT_QT_H

#include "ep/cpp/component/component.h"

#include "driver/qt/epqt.h"

#include <QObject>

namespace ep {
class Kernel;
}

namespace qt
{

SHARED_CLASS(QtComponent);

// This shim class wraps a QObject in an ep::Component that is accessible from the EP component system
class QtComponent : public Component
{
  EP_DECLARE_COMPONENT(QtComponent, Component, EPKERNEL_PLUGINVERSION, "QtComponent desc...")
public:

  QtComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~QtComponent();

  QObject *GetQObject() const { return pQObject; }

private:
  QObject *pQObject;
};

} // namespace qt

#endif // EP_QTCOMPONENT_QT_H
