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
#include <QtCore/private/qmetaobject_p.h>
#include <QtCore/private/qmetaobjectbuilder_p.h>

namespace qt {

using ep::RefCounted;
using ep::Variant;
using ep::String;
using ep::SharedString;
using ep::Array;
using ep::Slice;


// ---------------------------------------------------------------------------------------
// COMPONENT DESCRIPTOR SHIMS

// Property Data object - stored alongside the getter/setter methodshims
// This identifies which QObject property the EP property refers to
class QtPropertyData : public RefCounted
{
public:
  QtPropertyData(int _propertyId) : propertyId(_propertyId) {}
  int propertyId;
};

// Method Data object - stored alongside the call methodshim
// This identifies which QObject method the EP meta method refers to
class QtMethodData : public RefCounted
{
public:
  QtMethodData(int _methodId) : methodId(_methodId) {}
  int methodId;
};

// Event Data object - stored alongside the subscribe methodshim
// This identifies which QObject signal the EP event refers to and keeps track of the current list of subscribers
class QtEventData : public RefCounted
{
public:
  QtEventData(int signalId, SharedString signalName) : signalMapper(signalId, signalName) {}
  mutable QtSignalMapper signalMapper;
};


// This class is the conduit via which all meta system calls go from the Euclideon Platform system into QML
// Component Descriptors that map to an underlying QML based component will use this classes methods for meta access
class QtShims
{
public:
  // Property Getter
  Variant getter(Slice<const Variant>, const RefCounted &_data)
  {
    const QtPropertyData &data = (const QtPropertyData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();
    return Variant(pQObject->metaObject()->property(data.propertyId).read(pQObject));
  }

  // Property Setter
  Variant setter(Slice<const Variant> args, const RefCounted &_data)
  {
    const QtPropertyData &data = (const QtPropertyData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();
    pQObject->metaObject()->property(data.propertyId).write(pQObject, args[0].as<QVariant>());
    return Variant();
  }

  // Meta Method Call
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
    pQObject->metaObject()->method(data.methodId).invoke(pQObject, Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
      args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);

    return Variant(retVal);
  }

  // Event Subscription
  Variant subscribe(Slice<const Variant> values, const RefCounted &_data)
  {
    const QtEventData &data = (const QtEventData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->GetUserData();

    return Variant(data.signalMapper.Subscribe(pQObject, values[0].asDelegate()));
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

    SharedString propertyName = epFromQString(property.name()).toLower();
    auto data = SharedPtr<QtPropertyData>::create(i);
    auto getterShim = (property.isReadable() ? MethodShim(&QtShims::getter, data) : MethodShim(nullptr));
    auto setterShim = (property.isWritable() ? MethodShim(&QtShims::setter, data) : MethodShim(nullptr));
    uint32_t flags = (property.isConstant() ? ep::PropertyFlags::epPF_Immutable : 0);

    pDesc->propertyTree.Insert(propertyName, PropertyDesc(PropertyInfo{ propertyName, propertyName, propertyDescStr, nullptr, flags }, getterShim, setterShim));
  }

  for (int i = pMetaObject->methodOffset(); i < pMetaObject->methodCount(); ++i)
  {
    // Inject the methods
    QMetaMethod method = pMetaObject->method(i);
    if (method.methodType() == QMetaMethod::Slot)
    {
      static SharedString methodDescStr("Qt Component Method");

      SharedString methodName = epFromQString(method.name()).toLower();
      auto data = SharedPtr<QtMethodData>::create(i);
      auto shim = MethodShim(&QtShims::call, data);

      pDesc->methodTree.Insert(methodName, MethodDesc(MethodInfo{ methodName, methodDescStr }, shim));
    }
    // Inject the events
    else if (method.methodType() == QMetaMethod::Signal)
    {
      static SharedString eventDescStr("Qt Component Event");

      SharedString eventName = epFromQString(method.name()).toLower();
      auto data = SharedPtr<QtEventData>::create(i, eventName);
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
  return res;
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
  return res;
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
  return res;
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
  return res;
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
  return res;
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
  return res;
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
  return res;
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
  return res;
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
  return res;
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
  return res;
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
  return res;
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

  return res;
}

QVariant QtEPComponent::subscribe(QString eventName, QJSValue func) const
{
  EPASSERT(pComponent, "QtEPComponent contains a null component");
  if (!func.isCallable())
  {
    pComponent->LogError("Must subscribe to a javascript function. '{0}' is not callable.", func.toString());
    return QVariant();
  }

  QByteArray byteArray = eventName.toUtf8();
  String event(byteArray.data(), byteArray.size());
  QVariant res;
  try
  {
    epFromVariant(pComponent->Subscribe(event, ep::VarDelegate(JSValueDelegateRef::create(func))), &res);
  }
  catch (ep::EPException &)
  {
    ep::ClearError();
  }
  return res;
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


// Our internal QMetaObject - we can stuff extra info in here if necessary
struct QtEPMetaObject : public QMetaObject
{
  QtEPMetaObject()
  {
    d.data = 0;
    d.stringdata = 0;
  }

  ~QtEPMetaObject()
  {
    delete[](int*)d.data;
    delete[](char*)d.stringdata;
  }
};

// Helper factory class which creates a new QMetaObject based on a EP Component Descriptor
// TODO: This should cache meta objects for previously encountered types
class QtMetaObjectGenerator
{
public:
  // TODO
  static const QtEPMetaObject *CheckCache(const ep::ComponentDesc *pDesc) { return nullptr; }

  static const QtEPMetaObject *Generate(const ep::ComponentDesc *pDesc);

private:
  QtMetaObjectGenerator(const ep::ComponentDesc *pDesc);
  ~QtMetaObjectGenerator() {}

  QtEPMetaObject *CreateMetaObject(const QMetaObject *pParent);

  void AddProperty(const ep::PropertyDesc *pProperty);

  // TODO: potentially use this in the property map if we need further data
  //struct Property
  //{
    //int type = 0;
    //uint flags = 0;
  //};
  QMap<QByteArray, uint> propertyMap;
  const ep::ComponentDescInl *pDesc;
};


// Generate a custom QMetaObject that is populated based on the EP Component Descriptor and reflects the inheritance structure of the EP Component
// Note that QObject will be the always be the top most parent, followed by ep::Component and so forth.
const QtEPMetaObject *QtMetaObjectGenerator::Generate(const ep::ComponentDesc *pDesc)
{
  const QMetaObject *pParentObject = &QObject::staticMetaObject;
  const QtEPMetaObject *pMetaObj = nullptr;

  if (pMetaObj = QtMetaObjectGenerator::CheckCache(pDesc))
    return pMetaObj;

  else if (pDesc->pSuperDesc)
    pParentObject = QtMetaObjectGenerator::Generate(pDesc->pSuperDesc);

  QtMetaObjectGenerator generator(pDesc);
  return generator.CreateMetaObject(pParentObject);
}

// Internal only
QtMetaObjectGenerator::QtMetaObjectGenerator(const ep::ComponentDesc *_pDesc)
  : pDesc((const ep::ComponentDescInl*)_pDesc)
{
  using namespace ep;

  // Populate our property map with the component descriptor's properties (not including its super)
  for (auto p : pDesc->propertyTree)
  {
    if (!pDesc->pSuperDesc || !static_cast<const ep::ComponentDescInl*>(pDesc->pSuperDesc)->propertyTree.Get(p.key))
      AddProperty(&p.value);
  }

  // TODO: methods and events
}

// Internal only
QtEPMetaObject *QtMetaObjectGenerator::CreateMetaObject(const QMetaObject *pParent)
{
  // Verify MOC hasn't changed
  static_assert(QMetaObjectPrivate::OutputRevision == 7, "QtMetaObjectGenerator should generate the same version as moc; this code may need to be updated in order to be binary compatible!!");

  // QMetaObject's refer to two main variable length data blobs:
  // a) const uint *data - This provides the index, type, flag etc information for the property/method/classinfo/signals etc in addition to the header data
  // b) const QByteArrayData *stringdata - This provides the string data for property/method signatures etc.
  // NOTE!! Generating this information requires access to a couple private qt headers (contained within core-private), since private Qt classes don't adhere to
  // strict API conventions, these are subject to change and may require recompilation and/or code changes in order to be compatible with future releases.
  // However since these classes are used extensively by Qt's internal meta system in addition to MOC itself, it is unlikely for this to change without notice
  // and especially unlikely to occur in the 5.x series.

  QtEPMetaObject *pMetaObj = epNew(QtEPMetaObject);

  int paramsDataSize = 0; // TODO

  // Work out the size of the uint data block and allocate
  uint intDataSize = MetaObjectPrivateFieldCount; // base header
  intDataSize += propertyMap.count() * 3;         // property field
  ++intDataSize;                                  // eod marker
  uint *pIntData = epAllocType(uint, intDataSize, epAF_Zero);

  // Populate the data block header - this is internally represented as a QMetaObjectPrivate struct
  QMetaObjectPrivate *pHeader = reinterpret_cast<QMetaObjectPrivate *>(pIntData);
  pHeader->revision = QMetaObjectPrivate::OutputRevision;
  pHeader->className = 0;
  pHeader->classInfoCount = 0;
  pHeader->classInfoData = MetaObjectPrivateFieldCount;
  pHeader->methodCount = 0;
  pHeader->methodData = pHeader->classInfoData + pHeader->classInfoCount * 2;
  pHeader->propertyCount = propertyMap.count();
  pHeader->propertyData = pHeader->methodData + pHeader->methodCount * 5 + paramsDataSize;
  pHeader->enumeratorCount = 0;
  pHeader->enumeratorData = pHeader->propertyData + pHeader->propertyCount * 3;
  pHeader->constructorCount = 0;
  pHeader->constructorData = 0;
  pHeader->flags = 0;
  pHeader->signalCount = 0;

  // The QMetaStringTable will be used to generate the QMetaObject's stringdata block
  QMetaStringTable metaStringTable(QByteArray(pDesc->info.identifier.ptr, (int)pDesc->info.identifier.length));
  uint offset = pHeader->classInfoData;

  // TODO: methods

  // Populate the property data
  for (QMap<QByteArray, uint>::ConstIterator i = propertyMap.begin(); i != propertyMap.end(); ++i) {
    pIntData[offset++] = metaStringTable.enter(i.key());  // Property name
    pIntData[offset++] = QMetaType::QVariant;             // Property type
    pIntData[offset++] = i.value();                       // Property flags
  }

  EPASSERT_THROW(offset == (uint)pHeader->enumeratorData, epR_Failure, "Malformed QMetaObject");
  EPASSERT_THROW(offset == intDataSize - 1, epR_Failure, "Malformed QMetaObject");

  // eod marker
  pIntData[offset] = 0;

  // Allocate the stringdata block and populate from our QMetaStringTable object
  char *stringData = epAllocType(char, metaStringTable.blobSize(), epAF_None);
  metaStringTable.writeBlob(stringData);

  // Put the MetaObject together
  pMetaObj->d.data = pIntData;
  pMetaObj->d.extradata = 0;
  pMetaObj->d.stringdata = reinterpret_cast<const QByteArrayData *>(stringData);
  pMetaObj->d.static_metacall = 0;
  pMetaObj->d.relatedMetaObjects = 0;
  pMetaObj->d.superdata = pParent;

  return pMetaObj;
}

// Internal only
void QtMetaObjectGenerator::AddProperty(const ep::PropertyDesc *pProperty)
{
  uint &propFlags = propertyMap[QByteArray(pProperty->id.ptr, (int)pProperty->id.length)];
  propFlags = PropertyFlags::Designable | PropertyFlags::Scriptable;

  if (pProperty->getter) propFlags |= PropertyFlags::Readable;
  if (pProperty->setter) propFlags |= (PropertyFlags::Writable | PropertyFlags::Stored);
  if (pProperty->flags & ep::PropertyFlags::epPF_Immutable) propFlags |= PropertyFlags::Constant;

  // TODO: notify signal
}

// !! BEGIN INTERNAL MOC STUFF

// Unused - we override this with our fabricated object
const QMetaObject QtTestComponent::staticMetaObject = {
  { 0, 0, 0, 0, 0, 0 }
};

void QtTestComponent::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
  qt_static_metacall_int(qobject_cast<QtTestComponent*>(_o), _c, _id, _a);
}

const QMetaObject *QtTestComponent::metaObject() const
{
  if (!pMetaObj)
  {
    pMetaObj = QtMetaObjectGenerator::Generate(pComponent->GetDescriptor());
    EPASSERT_THROW(pMetaObj, epR_Failure, "Unable to generate QMetaObject for component type '{0}'", pComponent->GetType());
  }

  return pMetaObj;
}

void *QtTestComponent::qt_metacast(const char *cname)
{
  if (!qstrcmp(cname, "QtTestComponent")) return (void*)this;
  return QObject::qt_metacast(cname);
}

int QtTestComponent::qt_metacall(QMetaObject::Call call, int id, void **v)
{
  id = QObject::qt_metacall(call, id, v);
  if (id < 0)
    return id;

  const QMetaObject *mo = metaObject();

  switch (call)
  {
    case QMetaObject::InvokeMetaMethod:
      id = qt_static_metacall_int(this, call, id, v);
      break;
    case QMetaObject::ReadProperty:
    case QMetaObject::WriteProperty:
    case QMetaObject::ResetProperty:
      id = qt_metacall_property_int(call, id, v);
      break;
    case QMetaObject::QueryPropertyScriptable:
    case QMetaObject::QueryPropertyDesignable:
    case QMetaObject::QueryPropertyStored:
    case QMetaObject::QueryPropertyEditable:
    case QMetaObject::QueryPropertyUser:
      id -= mo->propertyCount();
      break;
    default:
      break;
  }

  EPASSERT_THROW(id < 0, epR_Failure, "qt_metacall called with unknown index '{0}' call type '{1}'", id, (int)call);
  return id;
}

// !! END INTERNAL MOC STUFF


QtTestComponent::QtTestComponent(ep::Component *pComp)
{
  EPASSERT_THROW(pComp, epR_InvalidArgument, "Creating a QtTestComponent from a null component");
  pComponent = pComp;
}

QtTestComponent::~QtTestComponent()
{
  // TODO: when these are cached, we won't need to do this
  while (pMetaObj && pMetaObj != &QObject::staticMetaObject)
  {
    const QMetaObject *pSuper = pMetaObj->superClass();
    epDelete((QtEPMetaObject*)pMetaObj);
    pMetaObj = pSuper;
  }
}

int QtTestComponent::qt_static_metacall_int(QtTestComponent *_tc, QMetaObject::Call _c, int _id, void **_a)
{
  // TODO: currently unused
  Q_ASSERT(_tc != 0);
  if (_c == QMetaObject::InvokeMetaMethod) {
    const QMetaObject *mo = _tc->metaObject();
    switch (mo->method(_id + mo->methodOffset()).methodType()) {
      case QMetaMethod::Signal:
        //QMetaObject::activate(_tc->qObject(), mo, _id, _a);
        //return _id - mo->methodCount();
      case QMetaMethod::Method:
      case QMetaMethod::Slot:
        //return _tc->internalInvoke(_c, _id, _a);
      default:
        break;
    }
  }
  return 0;
}

int QtTestComponent::qt_metacall_property_int(QMetaObject::Call call, int index, void **v)
{
  const QMetaObject *mo = metaObject();
  QMetaProperty prop = mo->property(index + QObject::staticMetaObject.propertyCount());
  index -= mo->propertyCount();

  switch (call)
  {
    case QMetaObject::ReadProperty:
    {
      ep::Variant var = pComponent->Get(prop.name());
      epFromVariant(var, (QVariant*)*v);
      break;
    }
    case QMetaObject::WriteProperty:
    {
      ep::Variant var;
      int typeId = prop.userType();
      if (typeId == (int)QMetaType::QVariant)
        var = epToVariant(*(QVariant*)v[0]);
      else
        var = epToVariant(QVariant(typeId, v[0]));
      pComponent->Set(prop.name(), var);
      break;
    }
    default:
      break;
  }

  return index;
}

} // namespace qt

#endif
