#pragma once
#ifndef EP_QTCOMPONENT_QT_H
#define EP_QTCOMPONENT_QT_H

#include <QObject>

#include "ep/cpp/component.h"

namespace ep {
class Kernel;
}

namespace qt
{

SHARED_CLASS(QtComponent);

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
