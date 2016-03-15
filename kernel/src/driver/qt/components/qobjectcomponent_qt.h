#pragma once
#ifndef EP_QOBJECTCOMPONENT_QT_H
#define EP_QOBJECTCOMPONENT_QT_H

#include "ep/cpp/component/component.h"

#include "components/dynamiccomponent.h"

#include "driver/qt/epqt.h"

class QObject;
class QmlComponentData;

namespace qt {

SHARED_CLASS(QObjectComponent);

// This shim class wraps a QObject in an ep::Component that is accessible from the EP component system
class QObjectComponent : public ep::DynamicComponent
{
  EP_DECLARE_COMPONENT(QObjectComponent, ep::DynamicComponent, EPKERNEL_PLUGINVERSION, "QObjectComponent desc...", 0)
public:
  QObjectComponent(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  virtual ~QObjectComponent();

  QObject *GetQObject() const { return pQObject; }
  template <class T>
  T *GetQObject() const { return qobject_cast<T*>(pQObject); }

private:
  friend class QtKernel;

  void AttachToGlue(Component *pGlue) override final;
  void SetupQObject();

  QObject *pQObject = nullptr;
  bool hasOwnership = false;
  QmlComponentData *pQmlComponentData = nullptr;
};

} // namespace qt

#endif // EP_QOBJECTCOMPONENT_QT_H
