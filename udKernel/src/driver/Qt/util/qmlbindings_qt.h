#pragma once
#ifndef QTCOMPONENT_H
#define QTCOMPONENT_H

#include <QObject>
#include <QQmlComponent>

#include "../udQtKernel_Internal.h"
#include "signaltodelegate.h"
#include "typeconvert.h"

#include "components/component.h"

namespace qt
{

// Qt shims
template<typename ComponentType>
struct QtGetter : public ud::Getter
{
public:
  QtGetter(udString id) : ud::Getter(nullptr), propertyId(id)
  {
    shim = &shimFunc;
  }

protected:
  udString propertyId;

  static udVariant shimFunc(const ud::Getter * const _pGetter, const ud::Component *pThis)
  {
    QtGetter *pGetter = (QtGetter*)_pGetter;
    const QObject *pQObject = (const QObject*)((const ComponentType*)pThis)->GetInternalData();
    return udVariant(pQObject->property(pGetter->propertyId.ptr));
  }
};


template<typename ComponentType>
struct QtSetter : public ud::Setter
{
public:
  QtSetter(udString id) : ud::Setter(nullptr), propertyId(id)
  {
    shim = &shimFunc;
  }

protected:
  udString propertyId;

  static void shimFunc(const ud::Setter * const _pSetter, ud::Component *pThis, const udVariant &value)
  {
    QtSetter *pSetter = (QtSetter*)_pSetter;
    QObject *pQObject = (QObject*)((ComponentType*)pThis)->GetInternalData();
    pQObject->setProperty(pSetter->propertyId.ptr, value.as<QVariant>());
  }
};

template<typename ComponentType>
struct QtMethod : public ud::Method
{
public:
  QtMethod(const QMetaMethod &_method) : ud::Method(nullptr), method(_method)
  {
    shim = &shimFunc;
  }

protected:
  QMetaMethod method;

  static udVariant shimFunc(const Method * const _pMethod, ud::Component *pThis, udSlice<udVariant> value)
  {
    QtMethod *pMethod = (QtMethod*)_pMethod;
    QObject *pQObject = (QObject*)((ComponentType*)pThis)->GetInternalData();

    // TODO: do better runtime handling of this rather than assert since this can come from the user - check against Q_METAMETHOD_INVOKE_MAX_ARGS
    // TODO: error output??
    UDASSERT(value.length <= 10, "Attempting to call method shim with more than 10 arguments");

    // TODO: Do something less ugly - list of QGenericArguments?
    // TODO: check value length against function arg length minus default args amount - need to parse the signature to get the default arg list?

    QGenericArgument args[10];
    for (int i = 0; i<10; ++i)
    {
      if (i < value.length)
        args[i] = value[i].as<QGenericArgument>();
      else
        args[i] = ::QGenericArgument();
    }

    QVariant retVal;
    pMethod->method.invoke(pQObject, Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
       args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);

    return udVariant(retVal);
  }
};

template<typename ComponentType>
struct QtVarEvent : public ud::VarEvent
{
public:
  QtVarEvent(const QMetaMethod &m) : ud::VarEvent(nullptr), method(m), sigToDel(nullptr)
  {
    pSubscribe = &doSubscribe;
  }

protected:
  QMetaMethod method;
  QtSignalToDelegate *sigToDel;

  static void doSubscribe(const VarEvent *pEv, const ud::ComponentRef &c, const udVariant::VarDelegate &d)
  {
    QtVarEvent *pEvent = (QtVarEvent*)pEv;
    ComponentType *pC = (ComponentType*)c.ptr();

    // TODO: hook up disconnect path
    // TODO: store list of signal to delegates
    pEvent->sigToDel = new QtSignalToDelegate((QObject*)pC->GetInternalData(), pEvent->method, d);
  }
};


template<typename ComponentType>
inline void PopulateComponentDesc(ComponentType *pComponent, QObject *pObject)
{
  // TODO: add built-in properties, methods and events

  const QMetaObject *pMetaObject = pObject->metaObject();

  // Inject the properties
  for (int i = 0; i < pMetaObject->propertyCount(); ++i)
  {
    // TODO: move this to class
    static char propertyDescStr[] = "Qt Component Property";

    QMetaProperty property = pMetaObject->property(i);

    // TODO: keep a list of string names that we manage so we can free
    // TODO: type & flags
    udString propertyName = AllocUDStringFromQString(property.name());
    ud::PropertyInfo info = { propertyName, propertyName, propertyDescStr };

    // TODO: have non lookup qmlproperty version and lookup version for the dynamic properties
    // TODO: list of qmlproperty - shared between getter and setter
    // TODO: store list to free getter/setter?
    ud::PropertyDesc desc = { info, udNew(QtGetter<ComponentType>, propertyName), udNew(QtSetter<ComponentType>, propertyName) };

    pComponent->AddDynamicProperty(desc);
  }

  // TODO: Inject the dynamic properties?
  // TODO: hook up to new property signal
  //foreach(const QByteArray &propName, pObject->dynamicPropertyNames())
  //{
  //qDebug() << "DYNAMIC PROPERTY " << propName;
  //}

  // TODO: Inject the methods
  for (int i = 0; i < pMetaObject->methodCount(); ++i)
  {
    QMetaMethod method = pMetaObject->method(i);
    if (method.methodType() == QMetaMethod::Slot)
    {
      // TODO: move this to class
      static char methodDescStr[] = "Qt Component Method";

      // TODO: keep list of strings
      // TODO: keep free list of methods
      udString methodName = AllocUDStringFromQString(method.name());
      ud::MethodInfo info = { methodName, methodDescStr };
      ud::MethodDesc desc = { info, udNew(QtMethod<ComponentType>, method) };

      pComponent->AddDynamicMethod(desc);
    }
    else if (method.methodType() == QMetaMethod::Signal)
    {
      // TODO: move this to class
      static char eventDescStr[] = "Qt Component Event";

      // TODO: keep list of strings
      // TODO: keep free list of events
      udString eventName = AllocUDStringFromQString(method.name());
      ud::EventInfo info = { eventName, eventName, eventDescStr };
      ud::EventDesc desc = { info, udNew(QtVarEvent<ComponentType>, method) };

      pComponent->AddDynamicEvent(desc);
    }
  }
}

} // namespace qt

#endif  // QTCOMPONENT_H
