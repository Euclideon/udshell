#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "qmlbindings_qt.h"
#include "signaltodelegate_qt.h"

namespace qt
{

class QtPropertyData : public RefCounted
{
public:
  QtPropertyData(String propertyId) : propertyId(propertyId) {}
  String propertyId;
};
class QtMethodData : public RefCounted
{
public:
  QtMethodData(QMetaMethod method) : method(method) {}
  QMetaMethod method;
};
class QtEventData : public RefCounted
{
public:
  QtEventData(QMetaMethod method) : method(method) {}
  QMetaMethod method;
  mutable Array<QtSignalToDelegate> sigToDel;
};

class QtShims
{
public:
  Variant getter(const QtPropertyData &data)
  {
    const QObject *pQObject = (const QObject*)((ep::Component*)this)->GetUserData();
    return Variant(pQObject->property(data.propertyId.toStringz()));
  }
  void setter(const QtPropertyData &data, const Variant &value)
  {
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();
    pQObject->setProperty(data.propertyId.toStringz(), value.as<QVariant>());
  }

  Variant call(const QtMethodData &data, Slice<const Variant> value)
  {
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();

    // TODO: do better runtime handling of this rather than assert since this can come from the user - check against Q_METAMETHOD_INVOKE_MAX_ARGS
    // TODO: error output??
    EPASSERT(value.length <= 10, "Attempting to call method shim with more than 10 arguments");

    // TODO: check value length against function arg length minus default args amount - need to parse the signature to get the default arg list?

    QVariant vargs[10];
    QGenericArgument args[10];
    for (size_t i = 0; i<10; ++i)
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
    data.method.invoke(pQObject, Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
                           args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);

    return Variant(retVal);
  }

  void subscribe(const QtEventData &data, const Variant::VarDelegate &d)
  {
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();

    // TODO: hook up disconnect path
    data.sigToDel.pushBack(QtSignalToDelegate(pQObject, data.method, d));
  }
};


void PopulateComponentDesc(kernel::Component *pComponent, QObject *pObject)
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
    PropertyInfo info = { propertyName, propertyName, propertyDescStr };

    // TODO: have non lookup qmlproperty version and lookup version for the dynamic properties
    // TODO: list of qmlproperty - shared between getter and setter
    // TODO: store list to free getter/setter?
    auto data = SharedPtr<QtPropertyData>::create(propertyName);

    auto getter = &QtShims::getter;
    auto setter = &QtShims::setter;
    kernel::PropertyDesc desc = { info, kernel::GetterShim(*(void**)&getter, data), kernel::SetterShim(*(void**)&setter, data) };

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
      MethodInfo info = { methodName, methodDescStr, nullptr };

      auto data = SharedPtr<QtMethodData>::create(method);

      auto call = &QtShims::call;
      kernel::MethodDesc desc = { info, kernel::MethodShim(*(void**)&call, data) };

      pComponent->AddDynamicMethod(desc);
    }
    else if (method.methodType() == QMetaMethod::Signal)
    {
      // TODO: move this to class
      static char eventDescStr[] = "Qt Component Event";

      // TODO: keep list of strings
      // TODO: keep free list of events
      SharedString eventName = AllocUDStringFromQString(method.name());
      EventInfo info = { eventName, eventName, eventDescStr, nullptr };

      auto data = SharedPtr<QtEventData>::create(method);

      auto subscribe = &QtShims::subscribe;
      kernel::EventDesc desc = { info, kernel::EventShim(*(void**)&subscribe, data) };

      pComponent->AddDynamicEvent(desc);
    }
  }
}

} // namespace qt

#endif
