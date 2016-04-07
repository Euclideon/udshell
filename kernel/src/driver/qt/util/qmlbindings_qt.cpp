#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "ep/cpp/componentdesc.h"
#include "ep/cpp/component/commandmanager.h"
#include "ep/cpp/component/broadcaster.h"
#include "components/lua.h"

#include "driver/qt/util/qmlbindings_qt.h"
#include "driver/qt/util/signaltodelegate_qt.h"
#include "driver/qt/components/qobjectcomponent_qt.h"

#include <QQmlContext>

namespace qt {

using ep::RefCounted;
using ep::Variant;
using ep::String;
using ep::SharedString;
using ep::Array;
using ep::Slice;

// ---------------------------------------------------------------------------------------
// COMPONENT DESCRIPTOR SHIMS
class QtPropertyData : public RefCounted
{
public:
  QtPropertyData(const QMetaProperty &_property) : property(_property) {}
  QMetaProperty property;
};

class QtMethodData : public RefCounted
{
public:
  QtMethodData(const QMetaMethod &_method) : method(_method) {}
  QMetaMethod method;
};

class QtEventData : public RefCounted
{
public:
  QtEventData(const QMetaMethod &_method) : method(_method) {}
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
    return Variant(data.property.read((const QObject*)((ep::Component*)this)->GetUserData()));
  }

  Variant setter(Slice<const Variant> args, const RefCounted &_data)
  {
    const QtPropertyData &data = (const QtPropertyData&)_data;
    data.property.write((QObject*)((ep::Component*)this)->GetUserData(), args[0].as<QVariant>());
    return Variant();
  }

  Variant call(Slice<const Variant> values, const RefCounted &_data)
  {
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
    const QtMethodData &data = (const QtMethodData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();
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
void PopulateComponentDesc(ep::ComponentDescInl *pDesc, QObject *pObject)
{
  using namespace ep;
  const QMetaObject *pMetaObject = pObject->metaObject();

  // Inject the properties
  for (int i = pMetaObject->propertyOffset(); i < pMetaObject->propertyCount(); ++i)
  {
    static SharedString propertyDescStr("Qt Component Property");
    QMetaProperty property = pMetaObject->property(i);

    SharedString propertyName = AllocUDStringFromQString(property.name()).toLower();
    auto data = SharedPtr<QtPropertyData>::create(property);
    auto getterShim = (property.isReadable() ? MethodShim(&QtShims::getter, data) : MethodShim(nullptr));
    auto setterShim = (property.isWritable() ? MethodShim(&QtShims::setter, data) : MethodShim(nullptr));

    pDesc->propertyTree.Insert(propertyName, PropertyDesc(PropertyInfo{ propertyName, propertyName, propertyDescStr }, getterShim, setterShim));
  }

  for (int i = pMetaObject->methodOffset(); i < pMetaObject->methodCount(); ++i)
  {
    // Inject the methods
    QMetaMethod method = pMetaObject->method(i);
    if (method.methodType() == QMetaMethod::Slot)
    {
      static SharedString methodDescStr("Qt Component Method");

      SharedString methodName = AllocUDStringFromQString(method.name()).toLower();
      auto data = SharedPtr<QtMethodData>::create(method);
      auto shim = MethodShim(&QtShims::call, data);

      pDesc->methodTree.Insert(methodName, MethodDesc(MethodInfo{ methodName, methodDescStr }, shim));
    }
    // Inject the events
    else if (method.methodType() == QMetaMethod::Signal)
    {
      static SharedString eventDescStr("Qt Component Event");

      SharedString eventName = AllocUDStringFromQString(method.name()).toLower();
      auto data = SharedPtr<QtEventData>::create(method);
      auto shim = EventShim(&QtShims::subscribe, data);

      pDesc->eventTree.Insert(eventName, EventDesc(EventInfo{ eventName, eventName, eventDescStr }, shim));
    }
  }
}

} // namespace internal

// Creates a new QObject based ep::DynamicComponent
ep::DynamicComponentRef QmlComponentData::CreateComponent(ep::KernelRef spKernel, Variant::VarMap initParams)
{
  // NOTE: we need to give QObjectComponent our 'this' pointer so it can do deferred QObject creation
  return spKernel->CreateComponent<QObjectComponent>({ { "qmlcomponentdata", (int64_t)(size_t)this } });
}

// Creates a QML Item from the stored QQmlComponent
QObject *QmlComponentData::CreateInstance(QQmlEngine *pQmlEngine, ep::Component *pGlueComponent)
{
  using namespace ep;

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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
  catch (ep::EPException &)
  {
    ep::ClearError();
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
    pComponent->Subscribe(event, ep::VarDelegate(JSValueDelegateRef::create(func)));
  }
  catch (ep::EPException &)
  {
    ep::ClearError();
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

QtEPComponent *QtKernelQml::createComponent(const QString typeId, QVariant initParams)
{
  EPASSERT(pKernel, "No active kernel");
  QByteArray byteArray = typeId.toUtf8();
  String typeString(byteArray.data(), byteArray.size());

  try
  {
    return BuildQtEPComponent::Create(pKernel->CreateComponent(typeString, epToVariant(initParams).asAssocArray()));
  }
  catch (ep::EPException &)
  {
    ep::ClearError();
    return nullptr;
  }
}

QtEPComponent *QtKernelQml::getCommandManager() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->GetCommandManager());
}

QtEPComponent *QtKernelQml::getLua() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->GetLua());
}

QtEPComponent *QtKernelQml::getStdOutBroadcaster() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->GetStdOutBroadcaster());
}

QtEPComponent *QtKernelQml::getStdErrBroadcaster() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->GetStdErrBroadcaster());
}

void QtKernelQml::exec(QString str)
{
  pKernel->Exec(String(str.toUtf8().data(), str.length()));
}

QtFocusManager *QtKernelQml::getFocusManager() const
{
  return pKernel->GetFocusManager();
}

} // namespace qt

#endif
