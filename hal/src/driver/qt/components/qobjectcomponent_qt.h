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

  QObject *GetQObject() const { return pQObject; }
  template <class T>
  T *GetQObject() const { return qobject_cast<T*>(pQObject); }

  const ep::PropertyDesc *GetPropertyDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final { return pThis->Component::GetPropertyDesc(_name, enumerateFlags); }
  const ep::MethodDesc *GetMethodDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final { return pThis->Component::GetMethodDesc(_name, enumerateFlags); }
  const ep::EventDesc *GetEventDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final { return pThis->Component::GetEventDesc(_name, enumerateFlags); }
  const ep::StaticFuncDesc *GetStaticFuncDesc(ep::String _name, ep::EnumerateFlags enumerateFlags = 0) const override final { return pThis->Component::GetStaticFuncDesc(_name, enumerateFlags); }

private:
  friend class QtKernel;

  void AttachToGlue(Component *pGlue) override final;

  QObject *pQObject = nullptr;
  bool hasOwnership = false;
  QmlComponentData *pQmlComponentData = nullptr;
};

} // namespace qt

#endif // EP_QOBJECTCOMPONENT_QT_H
