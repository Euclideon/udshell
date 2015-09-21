#pragma once
#ifndef UD_QTCOMPONENT_QT_H
#define UD_QTCOMPONENT_QT_H

#include <QObject>

#include "components/component.h"

class ud::Kernel;

namespace qt
{

class QtComponent : public ud::Component
{
  UD_COMPONENT(QtComponent);

public:
  QtComponent(const ud::ComponentDesc *pType, ud::Kernel *pKernel, udSharedString uid, udInitParams initParams);
  virtual ~QtComponent();

  QObject *GetQObject() const { return pQObject; }

private:
  QObject *pQObject;
};

} // namespace qt

#endif // UD_QTCOMPONENT_QT_H
