#pragma once
#ifndef QMLBINDINGS_QT_H
#define QMLBINDINGS_QT_H

#include <QQmlComponent>

#include "signaltodelegate_qt.h"
#include "typeconvert_qt.h"

#include "../components/qtcomponent_qt.h"

namespace qt
{

// Qt shims
template<typename ComponentType>
struct QtGetter : public Getter
{
public:
  QtGetter(String id) : Getter(nullptr), propertyId(id)
  {
    shim = &shimFunc;
  }

protected:
  String propertyId;

  static Variant shimFunc(const Getter * const _pGetter, const Component *pThis)
  {
    QtGetter *pGetter = (QtGetter*)_pGetter;
    const QObject *pQObject = (const QObject*)((const ComponentType*)pThis)->GetInternalData();
    return Variant(pQObject->property(pGetter->propertyId.toStringz()));
  }
};


template<typename ComponentType>
struct QtSetter : public Setter
{
public:
  QtSetter(String id) : Setter(nullptr), propertyId(id)
  {
    shim = &shimFunc;
  }

protected:
  String propertyId;

  static void shimFunc(const Setter * const _pSetter, Component *pThis, const Variant &value)
  {
    QtSetter *pSetter = (QtSetter*)_pSetter;
    QObject *pQObject = (QObject*)((ComponentType*)pThis)->GetInternalData();
    pQObject->setProperty(pSetter->propertyId.toStringz(), value.as<QVariant>());
  }
};

template<typename ComponentType>
struct QtMethod : public Method
{
public:
  QtMethod(const QMetaMethod &_method) : Method(nullptr), method(_method)
  {
    shim = &shimFunc;
  }

protected:
  QMetaMethod method;

  static Variant shimFunc(const Method * const _pMethod, Component *pThis, Slice<Variant> value)
  {
    QtMethod *pMethod = (QtMethod*)_pMethod;
    QObject *pQObject = (QObject*)((ComponentType*)pThis)->GetInternalData();

    // TODO: do better runtime handling of this rather than assert since this can come from the user - check against Q_METAMETHOD_INVOKE_MAX_ARGS
    // TODO: error output??
    EPASSERT(value.length <= 10, "Attempting to call method shim with more than 10 arguments");

    // TODO: check value length against function arg length minus default args amount - need to parse the signature to get the default arg list?

    QVariant vargs[10];
    QGenericArgument args[10];
    for (int i = 0; i<10; ++i)
    {
      if (i < value.length)
      {
        epFromVariant(value[i], &vargs[i]);
        args[i] = Q_ARG(QVariant, vargs[i]);
      }
      else
        args[i] = ::QGenericArgument();
    }

    QVariant retVal;
    pMethod->method.invoke(pQObject, Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
            args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);

    return Variant(retVal);
  }
};

template<typename ComponentType>
struct QtVarEvent : public VarEvent
{
public:
  QtVarEvent(const QMetaMethod &m) : VarEvent(nullptr), method(m), sigToDel(nullptr)
  {
    pSubscribe = &doSubscribe;
  }

protected:
  QMetaMethod method;
  QtSignalToDelegate *sigToDel;

  static void doSubscribe(const VarEvent *pEv, const ComponentRef &c, const Variant::VarDelegate &d)
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
    SharedString propertyName = AllocUDStringFromQString(property.name());
    ep::PropertyInfo info = { propertyName, propertyName, propertyDescStr };

    // TODO: have non lookup qmlproperty version and lookup version for the dynamic properties
    // TODO: list of qmlproperty - shared between getter and setter
    // TODO: store list to free getter/setter?
    ep::PropertyDesc desc = { info, udNew(QtGetter<ComponentType>, propertyName), udNew(QtSetter<ComponentType>, propertyName) };

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
      SharedString methodName = AllocUDStringFromQString(method.name());
      ep::FunctionInfo info = { methodName, methodDescStr };
      ep::MethodDesc desc = { info, udNew(QtMethod<ComponentType>, method) };

      pComponent->AddDynamicMethod(desc);
    }
    else if (method.methodType() == QMetaMethod::Signal)
    {
      // TODO: move this to class
      static char eventDescStr[] = "Qt Component Event";

      // TODO: keep list of strings
      // TODO: keep free list of events
      SharedString eventName = AllocUDStringFromQString(method.name());
      ep::EventInfo info = { eventName, eventName, eventDescStr };
      ep::EventDesc desc = { info, udNew(QtVarEvent<ComponentType>, method) };

      pComponent->AddDynamicEvent(desc);
    }
  }
}

} // namespace qt

#endif  // QMLBINDINGS_QT_H
