#pragma once
#ifndef EP_QTCOMPONENT_QT_H
#define EP_QTCOMPONENT_QT_H

#include <QObject>

#include "components/component.h"

class ep::Kernel;

namespace qt
{

class QtComponent : public ep::Component
{
  EP_COMPONENT(QtComponent);

public:
  QtComponent(const ep::ComponentDesc *pType, ep::Kernel *pKernel, epSharedString uid, epInitParams initParams);
  virtual ~QtComponent();

  QObject *GetQObject() const { return pQObject; }

private:
  QObject *pQObject;
};

} // namespace qt

#endif // EP_QTCOMPONENT_QT_H
