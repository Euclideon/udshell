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
  EP_DECLARE_COMPONENT(ep, QObjectComponent, ep::DynamicComponent, EPKERNEL_PLUGINVERSION, "QObjectComponent desc...", 0)
public:
  QObjectComponent(const ep::ComponentDesc *pType, ep::Kernel *pKernel, ep::SharedString uid, ep::Variant::VarMap initParams);
  virtual ~QObjectComponent();

  QObject *getQObject() const { return pQObject; }
  template <class T>
  T *getQObject() const { return qobject_cast<T*>(pQObject); }

  const ep::PropertyDesc *getPropertyDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final { return pThis->Component::getPropertyDesc(_name, enumerateFlags); }
  const ep::MethodDesc *getMethodDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final { return pThis->Component::getMethodDesc(_name, enumerateFlags); }
  const ep::EventDesc *getEventDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final { return pThis->Component::getEventDesc(_name, enumerateFlags); }
  const ep::StaticFuncDesc *getStaticFuncDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final { return pThis->Component::getStaticFuncDesc(_name, enumerateFlags); }

private:
  friend class QtKernel;

  void attachToGlue(Component *pGlue, ep::Variant::VarMap initParams) override final;

  QObject *pQObject = nullptr;
  bool hasOwnership = false;
  QmlComponentData *pQmlComponentData = nullptr;
};

} // namespace qt

#endif // EP_QOBJECTCOMPONENT_QT_H
