#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "ep/cpp/componentdesc.h"
#include "ep/cpp/component/commandmanager.h"

#include "driver/qt/util/qmlbindings_qt.h"
#include "driver/qt/util/signaltodelegate_qt.h"
#include "driver/qt/components/qobjectcomponent_qt.h"

#include <QQmlContext>

namespace qt {

// ---------------------------------------------------------------------------------------
// COMPONENT DESCRIPTOR SHIMS
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
  ~QtEventData() { /* free sigToDel... */ }
  QMetaMethod method;
  mutable Array<QtSignalToDelegate*> sigToDel;
};

class QtShims
{
public:
  Variant getter(Slice<const Variant>, const RefCounted &_data)
  {
    const QtPropertyData &data = (const QtPropertyData&)_data;
    const QObject *pQObject = (const QObject*)((ep::Component*)this)->GetUserData();
    return Variant(pQObject->property(data.propertyId.toStringz()));
  }

  Variant setter(Slice<const Variant> args, const RefCounted &_data)
  {
    const QtPropertyData &data = (const QtPropertyData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();
    pQObject->setProperty(data.propertyId.toStringz(), args[0].as<QVariant>());
    return Variant();
  }

  Variant call(Slice<const Variant> values, const RefCounted &_data)
  {
    const QtMethodData &data = (const QtMethodData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();

    // TODO: do better runtime handling of this rather than assert since this can come from the user - check against Q_METAMETHOD_INVOKE_MAX_ARGS
    // TODO: error output??
    EPASSERT(values.length <= 10, "Attempting to call method shim with more than 10 arguments");

    // TODO: check value length against function arg length minus default args amount - need to parse the signature to get the default arg list?

    QVariant vargs[10];
    QGenericArgument args[10];
    for (size_t i = 0; i<10; ++i)
    {
      if (i < values.length)
      {
        epFromVariant(values[i], &vargs[i]);
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

  Variant subscribe(Slice<const Variant> values, const RefCounted &_data)
  {
    const QtEventData &data = (const QtEventData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();

    data.sigToDel.pushBack(new QtSignalToDelegate(pQObject, data.method, values[0].asDelegate()));

    // TODO: return subscription handle of some sort
    return Variant();
  }
};

namespace internal {

// Helper function to dynamically populate the component descriptor with the QObject's meta data
void PopulateComponentDesc(Component *pComponent, QObject *pObject)
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

    auto getterShim = MethodShim(&QtShims::getter, data);
    auto setterShim = MethodShim(&QtShims::setter, data);
    pComponent->AddDynamicProperty(info, &getterShim, &setterShim);
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

      auto shim = MethodShim(&QtShims::call, data);
      pComponent->AddDynamicMethod(info, &shim);
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

      auto shim = EventShim(&QtShims::subscribe, data);
      pComponent->AddDynamicEvent(info, &shim);
    }
  }
}

} // namespace internal

// Creates a new QObject based ep::DynamicComponent
DynamicComponentRef QmlComponentData::CreateComponent(KernelRef spKernel, Variant::VarMap initParams)
{
  // NOTE: we need to give QObjectComponent our 'this' pointer so it can do deferred QObject creation
  return spKernel->CreateComponent<QObjectComponent>({ { "qmlcomponentdata", (int64_t)(size_t)this } });
}

// Creates a QML Item from the stored QQmlComponent
QObject *QmlComponentData::CreateInstance(QQmlEngine *pQmlEngine, Component *pGlueComponent)
{
  // check that we contain a valid QQmlComponent
  if (!qmlComponent.isReady())
  {
    // if the content was loaded on a different thread, block here until it's good to go
    while (qmlComponent.isLoading())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EPTHROW_IF(qmlComponent.isNull(), epR_InvalidType, "Attempting to create QML item from an empty QQmlComponent.");

    // error encountered with loading the file
    if (qmlComponent.isError())
    {
      // TODO: better error information/handling
      foreach(const QQmlError &error, qmlComponent.errors())
        pGlueComponent->LogError(SharedString::concat("QML Error: ", error.toString().toUtf8().data()));
      EPTHROW(epR_Failure, "Error compiling QML file");
    }
  }

  // create a new qml context specific to the qml item we're attempting to create
  QQmlContext *pContext = new QQmlContext(pQmlEngine->rootContext());
  epscope(fail) { delete pContext; };

  // create QObject wrapper for the enclosing EP Component
  qt::QtEPComponent *pEPComponent = BuildQtEPComponent::CreateWeak(pGlueComponent);
  epscope(fail) { delete pEPComponent; };

  // then expose it to the qml context
  pContext->setContextProperty("thisComponent", pEPComponent);

  // now create the actual qml item
  QObject *pQtObject = qmlComponent.create(pContext);
  if (!pQtObject)
  {
    // TODO: better error information/handling
    foreach(const QQmlError &error, qmlComponent.errors())
      pGlueComponent->LogError(SharedString::concat("QML Error: ", error.toString().toUtf8().data()));
    EPTHROW(epR_Failure, "Error creating QML Item");
  }

  // transfer ownership of our qt objects to ensure they are cleaned up
  pContext->setParent(pQtObject);
  pEPComponent->setParent(pQtObject);

  // TODO: remove this
  internal::PopulateComponentDesc(pGlueComponent, pQtObject);
  pEPComponent->Done(pQtObject);

  return pQtObject;
}


// ---------------------------------------------------------------------------------------
// QtEPComponent - ep::Component shim object into QML

QVariant QtEPComponent::get(const QString &name) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String prop(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(pComponent->Get(prop), &res);
  return std::move(res);
}

void QtEPComponent::set(const QString &name, QVariant val)
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String prop(byteArray.data(), byteArray.size());
  try
  {
    pComponent->Set(prop, epToVariant(val));
  }
  catch (EPException &)
  {
    ClearError();
  }
}

QVariant QtEPComponent::call(const QString &name) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, Slice<Variant>()), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1), epToVariant(arg2)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1), epToVariant(arg2), epToVariant(arg3)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1),
      epToVariant(arg2), epToVariant(arg3), epToVariant(arg4)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1),
      epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1),
      epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5), epToVariant(arg6)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1),
      epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5), epToVariant(arg6), epToVariant(arg7)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1),
      epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5), epToVariant(arg6),
      epToVariant(arg7), epToVariant(arg8)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }
  return std::move(res);
}

QVariant QtEPComponent::call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  QByteArray byteArray = name.toUtf8();
  String method(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Call(method, epToVariant(arg0), epToVariant(arg1),
      epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5),
      epToVariant(arg6), epToVariant(arg7), epToVariant(arg8), epToVariant(arg9)), &res);
  }
  catch (EPException &)
  {
    ClearError();
  }

  return std::move(res);
}

void QtEPComponent::subscribe(QString eventName, QJSValue func) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  if (!func.isCallable())
  {
    pComponent->LogError("Must subscribe to a javascript function. '{0}' is not callable.", func.toString());
    return;
  }

  QByteArray byteArray = eventName.toUtf8();
  String event(byteArray.data(), byteArray.size());

  try
  {
    pComponent->Subscribe(event, VarDelegate(JSValueDelegateRef::create(func)));
  }
  catch (EPException &)
  {
    ClearError();
  }
}


// ---------------------------------------------------------------------------------------
// QtKernelQml - QtKernel shim object into QML

QtEPComponent *QtKernelQml::findComponent(const QString &uid) const
{
  EPASSERT(pKernel, "No active kernel");
  QByteArray byteArray = uid.toUtf8();
  String uidString(byteArray.data(), byteArray.size());

  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->FindComponent(uidString));
}

QtEPComponent *QtKernelQml::createComponent(const QString typeId, QVariantMap initParams)
{
  EPASSERT(pKernel, "No active kernel");
  QByteArray byteArray = typeId.toUtf8();
  String typeString(byteArray.data(), byteArray.size());

  try
  {
    return BuildQtEPComponent::Create(pKernel->CreateComponent(typeString, epToVariant(initParams).asAssocArray()));
  }
  catch (EPException &)
  {
    ClearError();
    return nullptr;
  }
}

QtEPComponent *QtKernelQml::getCommandManager() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->GetCommandManager());
}

QtFocusManager *QtKernelQml::getFocusManager() const
{
  return pKernel->GetFocusManager();
}

} // namespace qt

#endif
