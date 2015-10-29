#pragma once
#ifndef EP_QTCOMPONENT_QT_H
#define EP_QTCOMPONENT_QT_H

#include <QObject>

#include "components/component.h"

class ep::Kernel;

namespace qt
{

PROTOTYPE_COMPONENT(QtComponent);

class QtComponent : public Component
{
  EP_COMPONENT(QtComponent);

public:
  QtComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  virtual ~QtComponent();

  QObject *GetQObject() const { return pQObject; }

private:
  QObject *pQObject;
};

} // namespace qt

#endif // EP_QTCOMPONENT_QT_H
