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

    if (pDesc->propertyTree.Get(propertyName))
    {
      QtApplication::Kernel()->LogWarning(1, "Property '{0}' already exists in parent component. Will be inaccessible outside of QML.");
      continue;
    }

    auto data = SharedPtr<QtPropertyData>::create(i);
    auto getterShim = (property.isReadable() ? MethodShim(&QtShims::getter, data) : MethodShim(nullptr));
    auto setterShim = (property.isWritable() ? MethodShim(&QtShims::setter, data) : MethodShim(nullptr));
    uint32_t flags = (property.isConstant() ? (uint32_t)ep::PropertyFlags::epPF_Immutable : 0);

    pDesc->propertyTree.Insert(propertyName, PropertyDesc(PropertyInfo{ propertyName, propertyName, propertyDescStr, nullptr, flags }, getterShim, setterShim));
  }

  for (int i = pMetaObject->methodOffset(); i < pMetaObject->methodCount(); ++i)
  {
    // Inject the methods
    QMetaMethod method = pMetaObject->method(i);
    SharedString name = epFromQString(method.name()).toLower();
    if (method.methodType() == QMetaMethod::Slot)
    {
      if (pDesc->methodTree.Get(name))
      {
        QtApplication::Kernel()->LogWarning(1, "Method '{0}' already exists in parent component. Will be inaccessible outside of QML.");
        continue;
      }

      static SharedString methodDescStr("Qt Component Method");

      auto data = SharedPtr<QtMethodData>::create(i);
      auto shim = MethodShim(&QtShims::call, data);

      pDesc->methodTree.Insert(name, MethodDesc(MethodInfo{ name, methodDescStr }, shim));
    }
    // Inject the events
    else if (method.methodType() == QMetaMethod::Signal)
    {
      if (pDesc->eventTree.Get(name))
      {
        QtApplication::Kernel()->LogWarning(1, "Event '{0}' already exists in parent component. Will be inaccessible outside of QML.");
        continue;
      }

      static SharedString eventDescStr("Qt Component Event");

      auto data = SharedPtr<QtEventData>::create(i, name);
      auto shim = EventShim(&QtShims::subscribe, data);

      pDesc->eventTree.Insert(name, EventDesc(EventInfo{ name, name, eventDescStr }, shim));
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
    epFree((uint*)d.data);
    epFree((char*)d.stringdata);
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
  const char *argNames[9] = { "arg0", "arg1", "arg2", "arg3", "arg4", "arg5", "arg6", "arg7", "arg8" };

  struct Method
  {
    uint flags = 0;
    uint returnType;
    ep::SharedArray<uint> paramTypes;
  };

  QtMetaObjectGenerator(const ep::ComponentDesc *pDesc);
  ~QtMetaObjectGenerator() {}

  QtEPMetaObject *CreateMetaObject(const QMetaObject *pParent);

  void AddProperty(const ep::PropertyDesc *pProperty);
  void AddSlot(const ep::MethodDesc *pMethod);
  void AddSignal(const ep::EventDesc *pEvent);

  int AggregateParamCount(const QMap<QByteArray, Method> &map);

  // TODO: potentially use this in the property map if we need further data
  //struct Property
  //{
    //int type = 0;
    //uint flags = 0;
  //};
  QMap<QByteArray, uint> propertyMap;
  QMap<QByteArray, Method> slotMap;
  QMap<QByteArray, Method> signalMap;

  const ep::ComponentDescInl *pDesc;
};


// Generate a custom QMetaObject that is populated based on the EP Component Descriptor and reflects the inheritance structure of the EP Component
// Note that QObject will be the always be the top most parent, followed by ep::Component and so forth.
const QtEPMetaObject *QtMetaObjectGenerator::Generate(const ep::ComponentDesc *pDesc)
{
  const QMetaObject *pParentObject = &QObject::staticMetaObject;
  const QtEPMetaObject *pMetaObj = nullptr;

  if ((pMetaObj = QtMetaObjectGenerator::CheckCache(pDesc)))
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

  // Populate our various meta maps with the component descriptor's info (not including its super)

  // Properties
  for (auto p : pDesc->propertyTree)
  {
    if (!pDesc->pSuperDesc || !static_cast<const ep::ComponentDescInl*>(pDesc->pSuperDesc)->propertyTree.Get(p.key))
      AddProperty(&p.value);
  }

  // Slots
  for (auto m : pDesc->methodTree)
  {
    if (!pDesc->pSuperDesc || !static_cast<const ep::ComponentDescInl*>(pDesc->pSuperDesc)->methodTree.Get(m.key))
      AddSlot(&m.value);
  }

  // Signals
  for (auto e : pDesc->eventTree)
  {
    if (!pDesc->pSuperDesc || !static_cast<const ep::ComponentDescInl*>(pDesc->pSuperDesc)->eventTree.Get(e.key))
      AddSignal(&e.value);
  }
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

  int paramsDataSize = ((AggregateParamCount(signalMap) + AggregateParamCount(slotMap)) * 2)  // Types and parameter names
                       - signalMap.count() - slotMap.count();                                 // Return parameters don't have names

  // Work out the size of the uint data block and allocate
  uint intDataSize = MetaObjectPrivateFieldCount;                             // base header
  intDataSize += (signalMap.count() + slotMap.count()) * 5 + paramsDataSize;  // slots/signals
  intDataSize += propertyMap.count() * 3;                                     // property field
  ++intDataSize;                                                              // eod marker
  uint *pIntData = epAllocType(uint, intDataSize, epAF_Zero);

  // Populate the data block header - this is internally represented as a QMetaObjectPrivate struct
  QMetaObjectPrivate *pHeader = reinterpret_cast<QMetaObjectPrivate *>(pIntData);
  pHeader->revision = QMetaObjectPrivate::OutputRevision;
  pHeader->className = 0;
  pHeader->classInfoCount = 0;
  pHeader->classInfoData = MetaObjectPrivateFieldCount;
  pHeader->methodCount = slotMap.count() + signalMap.count();
  pHeader->methodData = pHeader->classInfoData + pHeader->classInfoCount * 2;
  pHeader->propertyCount = propertyMap.count();
  pHeader->propertyData = pHeader->methodData + pHeader->methodCount * 5 + paramsDataSize;
  pHeader->enumeratorCount = 0;
  pHeader->enumeratorData = pHeader->propertyData + pHeader->propertyCount * 3;
  pHeader->constructorCount = 0;
  pHeader->constructorData = 0;
  pHeader->flags = 0;
  pHeader->signalCount = signalMap.count();

  // The QMetaStringTable will be used to generate the QMetaObject's stringdata block
  QMetaStringTable metaStringTable(QByteArray(pDesc->info.identifier.ptr, (int)pDesc->info.identifier.length));
  uint offset = pHeader->classInfoData;
  EPASSERT_THROW(offset == (uint)pHeader->methodData, epR_Failure, "Malformed QMetaObject");

  uint paramsOffset = offset + pHeader->methodCount * 5;
  // Populate the method data
  for (int x = 0; x < 2; ++x)
  {
    // Signals, then slots in order to match moc
    const QMap<QByteArray, Method> &map = (x == 0) ? signalMap : slotMap;
    for (QMap<QByteArray, Method>::ConstIterator it = map.begin(); it != map.end(); ++it)
    {
      size_t paramCount = it.value().paramTypes.length;
      pIntData[offset++] = metaStringTable.enter(it.key());       // Method name
      pIntData[offset++] = (uint)paramCount;                      // Method param count
      pIntData[offset++] = paramsOffset;                          // Method param offset
      pIntData[offset++] = metaStringTable.enter(QByteArray());   // Method tag
      pIntData[offset++] = it.value().flags;                      // Method flags

      // param types
      for (int i = -1; i < (int)paramCount; ++i)
        pIntData[paramsOffset++] = (i < 0) ? it.value().returnType : it.value().paramTypes[i];

      // param names
      for (int i = 0; i < (int)paramCount; ++i)
        pIntData[paramsOffset++] = metaStringTable.enter(QByteArray::fromRawData(argNames[i], sizeof(argNames[i])));
    }
  }

  EPASSERT_THROW(offset == (uint)(pHeader->methodData + pHeader->methodCount * 5), epR_Failure, "Malformed QMetaObject");
  EPASSERT_THROW(paramsOffset == (uint)pHeader->propertyData, epR_Failure, "Malformed QMetaObject");
  offset += paramsDataSize;
  EPASSERT_THROW(offset == (uint)(pHeader->propertyData), epR_Failure, "Malformed QMetaObject");

  // Populate the property data
  for (QMap<QByteArray, uint>::ConstIterator i = propertyMap.begin(); i != propertyMap.end(); ++i)
  {
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
// This will build the necessary Qt property information we need from an EP Property Descriptor
void QtMetaObjectGenerator::AddProperty(const ep::PropertyDesc *pProperty)
{
  uint &propFlags = propertyMap[QByteArray(pProperty->id.ptr, (int)pProperty->id.length)];
  propFlags = PropertyFlags::Designable | PropertyFlags::Scriptable;

  if (pProperty->getter) propFlags |= PropertyFlags::Readable;
  if (pProperty->setter) propFlags |= (PropertyFlags::Writable | PropertyFlags::Stored);
  if (pProperty->flags & ep::PropertyFlags::epPF_Immutable) propFlags |= PropertyFlags::Constant;

  // TODO: notify signal
}

// Internal only
// This will build the necessary Qt method/slot information we need from an EP Method Descriptor
void QtMetaObjectGenerator::AddSlot(const ep::MethodDesc *pMethod)
{
  Method &slot = slotMap[QByteArray(pMethod->id.ptr, (int)pMethod->id.length)];
  slot.flags = MethodFlags::AccessPublic | MethodFlags::MethodSlot;

  // Args
  if (!pMethod->argTypes.empty())
  {
    // TODO: should probably truncate and warn in this case?
    EPASSERT_THROW(pMethod->argTypes.length <= Q_METAMETHOD_INVOKE_MAX_ARGS, epR_Failure, "Qt only supports a maximum of {0} parameters", Q_METAMETHOD_INVOKE_MAX_ARGS);
    slot.paramTypes = ep::Array<uint>(ep::Alloc, pMethod->argTypes.length);
    for (size_t i = 0; i < slot.paramTypes.length; ++i)
      slot.paramTypes[i] = QMetaType::QVariant;
  }

  // Return type
  slot.returnType = QMetaType::QVariant;
}

// Internal only
// This will build the necessary Qt signal information we need from an EP Event Descriptor
void QtMetaObjectGenerator::AddSignal(const ep::EventDesc *pEvent)
{
  Method &signal = signalMap[QByteArray(pEvent->id.ptr, (int)pEvent->id.length)];
  signal.flags = MethodFlags::AccessPublic | MethodFlags::MethodSignal;

  // Args
  if (!pEvent->argTypes.empty())
  {
    // TODO: should probably truncate and warn in this case?
    EPASSERT_THROW(pEvent->argTypes.length <= Q_METAMETHOD_INVOKE_MAX_ARGS, epR_Failure, "Qt only supports a maximum of {0} parameters", Q_METAMETHOD_INVOKE_MAX_ARGS);
    signal.paramTypes = ep::Array<uint>(ep::Alloc, pEvent->argTypes.length);
    for (size_t i = 0; i < signal.paramTypes.length; ++i)
      signal.paramTypes[i] = QMetaType::QVariant;
  }

  signal.returnType = QMetaType::Void;
}

// Internal only
// Returns the sum of all parameters (including the return type) for the give method map
int QtMetaObjectGenerator::AggregateParamCount(const QMap<QByteArray, Method> &map)
{
  int sum = 0;
  for (QMap<QByteArray, Method>::ConstIterator it = map.begin(); it != map.end(); ++it)
    sum += (int)it->paramTypes.length + 1;
  return sum;
}

// !! BEGIN INTERNAL MOC STUFF

// Unused - we override this with our fabricated object
const QMetaObject QtTestComponent::staticMetaObject = {
  { 0, 0, 0, 0, 0, 0 }
};

// MOC function - static based meta access goes via here
void QtTestComponent::qt_static_metacall(QObject *pObj, QMetaObject::Call call, int id, void **v)
{
  QtTestComponent *pTC = qobject_cast<QtTestComponent*>(pObj);
  EPASSERT_THROW(pTC != 0, epR_Failure, "QObject is not of type QtTestComponent");

  const QMetaObject *pMO = pTC->metaObject();
  while (pMO->methodOffset() > id)
    pMO = pMO->superClass();

  switch (pMO->method(id).methodType())
  {
    case QMetaMethod::Signal:
      QMetaObject::activate(pTC, QMetaObjectPrivate::signalOffset(pMO), id - pMO->methodOffset(), v);
      break;
    case QMetaMethod::Method:
    case QMetaMethod::Slot:
      pTC->MethodInvoke(id - pMO->methodOffset(), v);
      break;
    default:
      break;
  }
}

// MOC function - retrieves the metaobject for an instance - used internally and externally
// This will lazily create the QMetaObject
const QMetaObject *QtTestComponent::metaObject() const
{
  if (!pMetaObj)
  {
    pMetaObj = QtMetaObjectGenerator::Generate(pComponent->GetDescriptor());
    EPASSERT_THROW(pMetaObj, epR_Failure, "Unable to generate QMetaObject for component type '{0}'", pComponent->GetType());
  }

  return pMetaObj;
}

// MOC function - used internally by qobject_cast
void *QtTestComponent::qt_metacast(const char *cname)
{
  if (!qstrcmp(cname, "QtTestComponent"))
    return (void*)this;
  return QObject::qt_metacast(cname);
}

// MOC function - instance based meta access goes via here
int QtTestComponent::qt_metacall(QMetaObject::Call call, int id, void **v)
{
  // See if this is a QObject meta call
  // Note that regular Qt behaviour is to try from the parent down, offsetting the id along the way
  // A negative id indicates the parent handled it
  id = QObject::qt_metacall(call, id, v);
  if (id < 0)
    return id;

  // This will spin up the meta object if we don't already have one
  const QMetaObject *pMO = metaObject();

  switch (call)
  {
    // Funnel meta method access
    case QMetaObject::InvokeMetaMethod:
    {
      switch (pMO->method(id + pMO->methodOffset()).methodType())
      {
        case QMetaMethod::Signal:
          QMetaObject::activate(this, pMO, id, v);
          id -= pMO->methodCount();
          break;
        case QMetaMethod::Method:
        case QMetaMethod::Slot:
          id = MethodInvoke(id, v);
          break;
        default:
          break;
      }
      break;
    }
    // Funnel meta property access
    case QMetaObject::ReadProperty:
    case QMetaObject::WriteProperty:
    case QMetaObject::ResetProperty:
      id = PropertyInvoke(call, id, v);
      break;
    // Don't do anything with queries but say we did
    case QMetaObject::QueryPropertyScriptable:
    case QMetaObject::QueryPropertyDesignable:
    case QMetaObject::QueryPropertyStored:
    case QMetaObject::QueryPropertyEditable:
    case QMetaObject::QueryPropertyUser:
      id -= pMO->propertyCount();
      break;
    default:
      break;
  }

  EPASSERT_THROW(id < 0, epR_Failure, "Unsupported Qt Meta Access with index '{0}' and call type '{1}'", id, (int)call);
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

// Internal
// Qt method access function
int QtTestComponent::MethodInvoke(int id, void **v)
{
  QMetaMethod method = pMetaObj->method(id + QObject::staticMetaObject.methodCount());
  id -= pMetaObj->methodCount();

  try {
    // Loop thru the parameters for the mapped signal and convert to ep::Variants
    Array<const Variant, Q_METAMETHOD_INVOKE_MAX_ARGS> varArgs(ep::Reserve, method.parameterCount());
    for (int i = 0; i < method.parameterCount(); ++i)
    {
      // Note that args[0] is the return value
      // We initialise a QVariant based on the parameter type - if the param is already a QVariant then pass it direct
      int type = method.parameterType(i);
      if (type == QMetaType::QVariant)
        varArgs.pushBack(epToVariant(*(QVariant*)v[i + 1]));
      else
        varArgs.pushBack(epToVariant(QVariant(type, v[i + 1])));
    }

    ep::Variant var = pComponent->Call(epFromQByteArray(method.name()), (Slice<const Variant>)varArgs);

    if (method.returnType() != QMetaType::Void)
      epFromVariant(var, (QVariant*)v[0]);
  }
  catch (ep::EPException &) {
    ep::ClearError();
  }

  return id;
}

// Internal
// Qt property access function
int QtTestComponent::PropertyInvoke(QMetaObject::Call call, int id, void **v)
{
  QMetaProperty prop = pMetaObj->property(id + QObject::staticMetaObject.propertyCount());
  id -= pMetaObj->propertyCount();

  switch (call)
  {
    case QMetaObject::ReadProperty:
    {
      try {
        epFromVariant(pComponent->Get(prop.name()), (QVariant*)*v);
      }
      catch (ep::EPException &) {
        ep::ClearError();
      }
      break;
    }
    case QMetaObject::WriteProperty:
    {
      try {
        pComponent->Set(prop.name(), epToVariant(*(QVariant*)v[0]));
      }
      catch (ep::EPException &) {
        ep::ClearError();
      }
      break;
    }
    default:
      EPASSERT_THROW(false, epR_Failure, "Unsupported meta property access of type '{0}'", (int)call);
      break;
  }

  return id;
}

void QtTestComponent::connectNotify(const QMetaMethod &signal)
{
  ep::MutableString<0> signalName = epFromQByteArray(signal.name());

  // Check if we already have an entry in the connection map
  if (connectionMap.Get(signalName))
    return;

  Connection *pConn = &connectionMap.Insert(signalName, Connection{ this, signal });
  pConn->subscription = pComponent->Subscribe(signalName, ep::VarDelegate(pConn, &QtTestComponent::Connection::SignalRouter));
}

void QtTestComponent::disconnectNotify(const QMetaMethod &signal)
{
  // If all signals have been disconnected, clean up everything
  if (!signal.isValid())
  {
    for (auto conn : connectionMap)
      conn.value.subscription->Unsubscribe();
    connectionMap.Clear();
  }

  // If this was the last receiver for this signal, then clean up this connection
  if (receivers(QByteArray::number(QSIGNAL_CODE) + signal.methodSignature()) == 0)
  {
    auto conn = connectionMap.find(epFromQByteArray(signal.name()));
    conn->subscription->Unsubscribe();
    connectionMap.erase(conn);
  }
}

ep::Variant QtTestComponent::Connection::SignalRouter(ep::Slice<const ep::Variant> args)
{
  // Setup the params
  // Note that argv[0] is the return value - for signals we wont have one
  QVariant qvars[Q_METAMETHOD_INVOKE_MAX_ARGS];
  void *argv[Q_METAMETHOD_INVOKE_MAX_ARGS + 1] = {0};
  for (int i = 0; i < signal.parameterCount(); ++i)
  {
    qvars[i] = args[i].as<QVariant>();
    argv[i+1] = &qvars[i];
  }

  if (!pComp->signalsBlocked())
    QtTestComponent::qt_static_metacall(pComp, QMetaObject::InvokeMetaMethod, signal.methodIndex(), argv);
  return nullptr;
}

} // namespace qt

#endif
