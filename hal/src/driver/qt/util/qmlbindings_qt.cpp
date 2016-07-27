#include "driver.h"

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
    QObject *pQObject = (QObject*)((ep::Component*)this)->getUserData();
    return Variant(pQObject->metaObject()->property(data.propertyId).read(pQObject));
  }

  // Property Setter
  Variant setter(Slice<const Variant> args, const RefCounted &_data)
  {
    const QtPropertyData &data = (const QtPropertyData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->getUserData();
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
    QObject *pQObject = (QObject*)((ep::Component*)this)->getUserData();
    pQObject->metaObject()->method(data.methodId).invoke(pQObject, Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
      args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);

    return Variant(retVal);
  }

  // Event Subscription
  Variant subscribe(Slice<const Variant> values, const RefCounted &_data)
  {
    const QtEventData &data = (const QtEventData&)_data;
    QObject *pQObject = (QObject*)((ep::Component*)this)->getUserData();

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

    SharedString propertyName = epFromQString(property.name());

    if (pDesc->propertyTree.get(propertyName))
    {
      QtApplication::kernel()->logWarning(1, "Property '{0}' already exists in parent component. Will be inaccessible outside of QML.", propertyName);
      continue;
    }

    auto data = SharedPtr<QtPropertyData>::create(i);
    auto getterShim = (property.isReadable() ? MethodShim(&QtShims::getter, data) : MethodShim(nullptr));
    auto setterShim = (property.isWritable() ? MethodShim(&QtShims::setter, data) : MethodShim(nullptr));
    uint32_t flags = (property.isConstant() ? (uint32_t)ep::PropertyFlags::epPF_Immutable : 0);

    pDesc->propertyTree.insert(propertyName, PropertyDesc(PropertyInfo{ propertyName, propertyDescStr, nullptr, flags }, getterShim, setterShim));
  }

  for (int i = pMetaObject->methodOffset(); i < pMetaObject->methodCount(); ++i)
  {
    // Inject the methods
    QMetaMethod method = pMetaObject->method(i);
    SharedString name = epFromQString(method.name());
    if (method.methodType() == QMetaMethod::Slot)
    {
      if (pDesc->methodTree.get(name))
      {
        QtApplication::kernel()->logWarning(1, "Method '{0}' already exists in parent component. Will be inaccessible outside of QML.", name);
        continue;
      }

      static SharedString methodDescStr("Qt Component Method");

      auto data = SharedPtr<QtMethodData>::create(i);
      auto shim = MethodShim(&QtShims::call, data);

      pDesc->methodTree.insert(name, MethodDesc(MethodInfo{ name, methodDescStr }, shim));
    }
    // Inject the events
    else if (method.methodType() == QMetaMethod::Signal)
    {
      if (pDesc->eventTree.get(name))
      {
        QtApplication::kernel()->logWarning(1, "Event '{0}' already exists in parent component. Will be inaccessible outside of QML.", name);
        continue;
      }

      static SharedString eventDescStr("Qt Component Event");

      auto data = SharedPtr<QtEventData>::create(i, name);
      auto shim = EventShim(&QtShims::subscribe, data);

      pDesc->eventTree.insert(name, EventDesc(EventInfo{ name, eventDescStr }, shim));
    }
  }
}

} // namespace internal

// Creates a new QObject based ep::DynamicComponent
ep::DynamicComponentRef QmlComponentData::CreateComponent(ep::KernelRef spKernel, Variant::VarMap initParams)
{
  // NOTE: we need to give QObjectComponent our 'this' pointer so it can do deferred QObject creation
  return spKernel->createComponent<QObjectComponent>({ { "qmlComponentData", (int64_t)(size_t)this } });
}

// Creates a QML Item from the stored QQmlComponent
QObject *QmlComponentData::createInstance(QQmlEngine *pQmlEngine, ep::Component *pGlueComponent, ep::Variant::VarMap initParams)
{
  using namespace ep;

  // check that we contain a valid QQmlComponent
  if (!qmlComponent.isReady())
  {
    // if the content was loaded on a different thread, block here until it's good to go
    while (qmlComponent.isLoading())
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    EPTHROW_IF(qmlComponent.isNull(), Result::InvalidType, "Attempting to create QML item from an empty QQmlComponent.");

    // error encountered with loading the file
    if (qmlComponent.isError())
    {
      // TODO: better error information/handling
      foreach(const QQmlError &error, qmlComponent.errors())
        pGlueComponent->logError(SharedString::concat("QML Error: ", error.toString().toUtf8().data()));
      EPTHROW(Result::Failure, "Error compiling QML file");
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
  QObject *pQtObject = qmlComponent.beginCreate(pContext);
  if (!pQtObject)
  {
    // TODO: better error information/handling
    foreach(const QQmlError &error, qmlComponent.errors())
      pGlueComponent->logError(SharedString::concat("QML Error: ", error.toString().toUtf8().data()));
    EPTHROW(Result::Failure, "Error creating QML Item");
  }

  // check if the qml has an init property method and if so, pass in the initparams
  // this lets us modify the state of the qml component according to the init params *before* onCompleted and property binding occurs
  int initPropId = pQtObject->metaObject()->indexOfMethod("epInitComponent(QVariant)");
  if (initPropId != -1)
    pQtObject->metaObject()->method(initPropId).invoke(pQtObject, Qt::DirectConnection, Q_ARG(QVariant, ep::Variant(initParams).as<QVariant>()));
  qmlComponent.completeCreate();

  // transfer ownership of our qt objects to ensure they are cleaned up
  pContext->setParent(pQtObject);
  pEPComponent->setParent(pQtObject);

  return pQtObject;
}


// ---------------------------------------------------------------------------------------
// QtKernelQml - QtKernel shim object into QML

QtEPComponent *QtKernelQml::findComponent(const QString &uid) const
{
  EPASSERT(pKernel, "No active kernel");
  QByteArray byteArray = uid.toUtf8();
  String uidString(byteArray.data(), byteArray.size());

  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->findComponent(uidString));
}

QtEPComponent *QtKernelQml::createComponent(const QString typeId, QVariant initParams)
{
  EPASSERT(pKernel, "No active kernel");
  QByteArray byteArray = typeId.toUtf8();
  String typeString(byteArray.data(), byteArray.size());

  try
  {
    return BuildQtEPComponent::Create(pKernel->createComponent(typeString, epToVariant(initParams).asAssocArray()));
  }
  catch (ep::EPException &)
  {
    return nullptr;
  }
}

QtEPComponent *QtKernelQml::getCommandManager() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->getCommandManager());
}

QtEPComponent *QtKernelQml::getLua() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->getLua());
}

QtEPComponent *QtKernelQml::getStdOutBroadcaster() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->getStdOutBroadcaster());
}

QtEPComponent *QtKernelQml::getStdErrBroadcaster() const
{
  EPASSERT(pKernel, "No active kernel");
  // TODO: this should default to JS ownership but doublecheck!!
  return BuildQtEPComponent::Create(pKernel->getStdErrBroadcaster());
}

void QtKernelQml::exec(QString str)
{
  pKernel->exec(String(str.toUtf8().data(), str.length()));
}

QtFocusManager *QtKernelQml::getFocusManager() const
{
  return pKernel->getFocusManager();
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

ep::HashMap<ep::SharedString, QtEPMetaObject*> QtMetaObjectGenerator::metaObjectCache;
ep::HashMap<ep::SharedString, QtMetaObjectGenerator::RunBuiltInFunc> QtMetaObjectGenerator::builtInMethods;
int QtMetaObjectGenerator::builtInOffset = -1;
int QtMetaObjectGenerator::builtInCount = 0;


// Generate a custom QMetaObject that is populated based on the EP Component Descriptor and reflects the inheritance structure of the EP Component
// Note that QObject will be the always be the top most parent, followed by ep::Component and so forth.
// This will cache the result in an internal meta object hashmap
const QtEPMetaObject *QtMetaObjectGenerator::Generate(const ep::ComponentDesc *pDesc)
{
  EPASSERT(pDesc, "Attempting to generate a Qt meta object from null descriptor");

  const QMetaObject *pParentObject = &QObject::staticMetaObject;
  QtEPMetaObject **ppMetaObj = metaObjectCache.get(pDesc->info.identifier);

  if (ppMetaObj)
    return *ppMetaObj;
  else if (pDesc->pSuperDesc)
    pParentObject = QtMetaObjectGenerator::Generate(pDesc->pSuperDesc);

  QtMetaObjectGenerator generator(pDesc);
  return metaObjectCache.insert(pDesc->info.identifier, generator.CreateMetaObject(pParentObject));
}

// Clear the internal cache
void QtMetaObjectGenerator::ClearCache()
{
  for (auto pMetaObj : metaObjectCache)
    epDelete(pMetaObj.value);
  metaObjectCache.clear();
}


// Internal only
QtMetaObjectGenerator::QtMetaObjectGenerator(const ep::ComponentDesc *_pDesc)
  : pDesc((const ep::ComponentDescInl*)_pDesc)
{
  using namespace ep;

  // If we're the top level component, then insert our built-ins
  if (!pDesc->pSuperDesc)
  {
    slotList.reserve(pDesc->methodTree.size() + 5);

    // TODO: update these when arg types are supported
    AddMethod(MethodType::Slot, "get", Slice<const SharedString>{ "propertyName" },
      [](QtEPComponent *pObj, ep::Slice<const ep::Variant> varArgs) -> QVariant {
        return pObj->pComponent->get(varArgs[0].asString()).as<QVariant>();
      });
    AddMethod(MethodType::Slot, "set", Slice<const SharedString>{ "propertyName", "value" },
      [](QtEPComponent *pObj, ep::Slice<const ep::Variant> varArgs) -> QVariant {
        pObj->pComponent->set(varArgs[0].asString(), varArgs[1]);
        return QVariant();
      });
    AddMethod(MethodType::Slot, "call", Slice<const SharedString>{ "methodName", "arg0", "arg1", "arg2", "arg3", "arg4", "arg5", "arg6", "arg7", "arg8" },
      [](QtEPComponent *pObj, ep::Slice<const ep::Variant> varArgs) -> QVariant {
        return pObj->pComponent->call(varArgs[0].asString(), varArgs.slice(1, varArgs.length)).as<QVariant>();
      }, 9);
    AddMethod(MethodType::Slot, "subscribe", Slice<const SharedString>{ "eventName", "delegate" },
      [](QtEPComponent *pObj, ep::Slice<const ep::Variant> varArgs) -> QVariant {
        return ep::Variant(pObj->pComponent->subscribe(varArgs[0].asString(), varArgs[1].asDelegate())).as<QVariant>();
      });
    AddMethod(MethodType::Slot, "disconnectAll", nullptr,
      [](QtEPComponent *pObj, ep::Slice<const ep::Variant> varArgs) -> QVariant {
        pObj->disconnect();
        return QVariant();
      });
  }

  // Populate our various meta maps with the component descriptor's info (not including its super)

  // ------------- Properties
  propertyList.reserve(pDesc->propertyTree.size());
  for (auto p : pDesc->propertyTree)
    if (!pDesc->pSuperDesc || !static_cast<const ep::ComponentDescInl*>(pDesc->pSuperDesc)->propertyTree.get(p.key))
      AddProperty(&p.value);

  // ------------- Slots
  slotList.reserve(pDesc->methodTree.size());
  for (auto m : pDesc->methodTree)
    if (!pDesc->pSuperDesc || !static_cast<const ep::ComponentDescInl*>(pDesc->pSuperDesc)->methodTree.get(m.key))
      AddSlot(&m.value);

  // ------------- Signals
  signalList.reserve(pDesc->eventTree.size());
  for (auto e : pDesc->eventTree)
    if (!pDesc->pSuperDesc || !static_cast<const ep::ComponentDescInl*>(pDesc->pSuperDesc)->eventTree.get(e.key))
      AddSignal(&e.value);

  // Signals are inserted before slots so we need to keep track of the offset if this is the top level component (and hence has built-ins)
  if (!pDesc->pSuperDesc)
    builtInOffset = (int)signalList.size();
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

  int paramsDataSize = ((AggregateParamCount(signalList) + AggregateParamCount(slotList)) * 2)  // Types and parameter names
                       - (int)(signalList.length + slotList.length);                            // Return parameters don't have names

  // Work out the size of the uint data block and allocate
  uint intDataSize = MetaObjectPrivateFieldCount;                                 // base header
  intDataSize += (int)(signalList.length + slotList.length) * 5 + paramsDataSize; // slots/signals
  intDataSize += (int)(propertyList.length * 3);                                  // property field
  ++intDataSize;                                                                  // eod marker
  uint *pIntData = epAllocType(uint, intDataSize, epAF_Zero);

  // Populate the data block header - this is internally represented as a QMetaObjectPrivate struct
  QMetaObjectPrivate *pHeader = reinterpret_cast<QMetaObjectPrivate *>(pIntData);
  pHeader->revision = QMetaObjectPrivate::OutputRevision;
  pHeader->className = 0;
  pHeader->classInfoCount = 0;
  pHeader->classInfoData = MetaObjectPrivateFieldCount;
  pHeader->methodCount = (int)(slotList.length + signalList.length);
  pHeader->methodData = pHeader->classInfoData + pHeader->classInfoCount * 2;
  pHeader->propertyCount = (int)propertyList.length;
  pHeader->propertyData = pHeader->methodData + pHeader->methodCount * 5 + paramsDataSize;
  pHeader->enumeratorCount = 0;
  pHeader->enumeratorData = pHeader->propertyData + pHeader->propertyCount * 3;
  pHeader->constructorCount = 0;
  pHeader->constructorData = 0;
  pHeader->flags = 0;
  pHeader->signalCount = (int)signalList.length;

  // The QMetaStringTable will be used to generate the QMetaObject's stringdata block
  QMetaStringTable metaStringTable(QByteArray(pDesc->info.identifier.ptr, (int)pDesc->info.identifier.length));
  uint offset = pHeader->classInfoData;
  EPASSERT(offset == (uint)pHeader->methodData, "Malformed QMetaObject");

  uint paramsOffset = offset + pHeader->methodCount * 5;
  // Populate the method data
  for (int x = 0; x < 2; ++x)
  {
    // Signals, then slots in order to match moc
    Slice<Method> methodList = (x == 0) ? signalList : slotList;
    for (auto &m : methodList)
    {
      size_t paramCount = m.paramTypes.length;
      pIntData[offset++] = metaStringTable.enter(m.name);         // Method name
      pIntData[offset++] = (uint)paramCount;                      // Method param count
      pIntData[offset++] = paramsOffset;                          // Method param offset
      pIntData[offset++] = metaStringTable.enter(QByteArray());   // Method tag
      pIntData[offset++] = m.flags;                               // Method flags

      // param types
      for (int i = -1; i < (int)paramCount; ++i)
        pIntData[paramsOffset++] = (i < 0) ? m.returnType : m.paramTypes[i];

      // param names
      for (size_t i = 0; i < paramCount; ++i)
        pIntData[paramsOffset++] = metaStringTable.enter(QByteArray::fromRawData(pArgNames[i], sizeof(pArgNames[i])));
    }
  }

  EPASSERT(offset == (uint)(pHeader->methodData + pHeader->methodCount * 5), "Malformed QMetaObject");
  EPASSERT(paramsOffset == (uint)pHeader->propertyData, "Malformed QMetaObject");
  offset += paramsDataSize;
  EPASSERT(offset == (uint)(pHeader->propertyData), "Malformed QMetaObject");

  // Populate the property data
  for (auto &p : propertyList)
  {
    pIntData[offset++] = metaStringTable.enter(p.name);   // Property name
    pIntData[offset++] = QMetaType::QVariant;             // Property type
    pIntData[offset++] = p.flags;                         // Property flags
  }

  EPASSERT(offset == (uint)pHeader->enumeratorData, "Malformed QMetaObject");
  EPASSERT(offset == intDataSize - 1, "Malformed QMetaObject");

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
  auto &prop = propertyList.pushBack();
  prop.name.setRawData(pProperty->id.ptr, (uint)pProperty->id.length);
  prop.flags = PropertyFlags::Designable | PropertyFlags::Scriptable;

  if (pProperty->getter) prop.flags |= PropertyFlags::Readable;
  if (pProperty->setter) prop.flags |= (PropertyFlags::Writable | PropertyFlags::Stored);
  if (pProperty->flags & ep::PropertyFlags::epPF_Immutable) prop.flags |= PropertyFlags::Constant;

  // TODO: notify signal
}

// Internal only
// This will build the necessary Qt method information we need from an EP Descriptor
void QtMetaObjectGenerator::AddMethod(MethodType type, String name, Slice<const SharedString> params, RunBuiltInFunc runMethod, int numDefaultArgs)
{
  // If we've supplied a runMethod, then this is a built-in
  if (runMethod)
  {
    builtInMethods.insert(name, runMethod);
    builtInCount += numDefaultArgs + 1;
  }

  bool duplicate = false;
  do
  {
    auto &method = (type == MethodType::Signal) ? signalList.pushBack() : slotList.pushBack();
    method.name.setRawData(name.ptr, (uint)name.length);
    method.flags = MethodFlags::AccessPublic | ((type == MethodType::Signal) ? MethodFlags::MethodSignal : MethodFlags::MethodSlot);
    if (duplicate)
      method.flags |= MethodFlags::MethodCloned;

    // Args
    if (!params.empty())
    {
      // TODO: should probably truncate and warn in this case?
      EPASSERT_THROW(params.length <= Q_METAMETHOD_INVOKE_MAX_ARGS, ep::Result::Failure, "Qt only supports a maximum of {0} parameters", Q_METAMETHOD_INVOKE_MAX_ARGS);
      method.paramTypes = ep::Array<uint>(ep::Alloc, params.length);
      for (size_t i = 0; i < method.paramTypes.length; ++i)
        method.paramTypes[i] = QMetaType::QVariant;
    }

    // TODO: convert this from the EP?
    method.returnType = (type == MethodType::Signal) ? QMetaType::Void : QMetaType::QVariant;

    // If we have specified default args then we need to generate multiple entries
    if (--numDefaultArgs < 0)
      break;

    duplicate = true;
    params.popBack();
  } while (true);
}

// Internal only
// Returns the sum of all parameters (including the return type) for the given method list
int QtMetaObjectGenerator::AggregateParamCount(Slice<Method> methodList)
{
  int sum = 0;
  for (auto &m : methodList)
    sum += (int)(m.paramTypes.length + 1);
  return sum;
}


// !! BEGIN INTERNAL MOC STUFF

#define QT_MOC_LITERAL(idx, ofs, len) { \
  Q_REFCOUNT_INITIALIZE_STATIC, len, 0, 0, offsetof(qt_meta_stringdata_QtEPComponent_t, stringdata) + ofs - idx * sizeof(QByteArrayData) \
}

struct qt_meta_stringdata_QtEPComponent_t {
  QByteArrayData data[1];
  char stringdata[18];
};

const qt_meta_stringdata_QtEPComponent_t qt_meta_stringdata_QtEPComponent = {
  {
    QT_MOC_LITERAL(0, 0, 17)
  },
  "qt::QtEPComponent"
};
#undef QT_MOC_LITERAL

const uint qt_meta_data_QtEPComponent[] = {
  // content:
  7,       // revision
  0,       // classname
  0,    0, // classinfo
  0,    0, // methods
  0,    0, // properties
  0,    0, // enums/sets
  0,    0, // constructors
  0,       // flags
  0,       // signalCount
  0        // eod
};

// Largely unused - we provide a basic static meta object for situations where the dynamic one wont get used
const QMetaObject QtEPComponent::staticMetaObject = {
  { &QObject::staticMetaObject, qt_meta_stringdata_QtEPComponent.data,
  qt_meta_data_QtEPComponent, qt_static_metacall, 0, 0 }
};


// MOC function - static based meta access goes via here
void QtEPComponent::qt_static_metacall(QObject *pObj, QMetaObject::Call call, int id, void **v)
{
  QtEPComponent *pC = qobject_cast<QtEPComponent*>(pObj);
  EPASSERT(pC != 0, "QObject is not of type QtTestComponent");

  const QMetaObject *pMO = pC->metaObject();
  while (pMO->methodOffset() > id)
    pMO = pMO->superClass();

  switch (pMO->method(id).methodType())
  {
    case QMetaMethod::Signal:
      QMetaObject::activate(pC, QMetaObjectPrivate::signalOffset(pMO), id - pMO->methodOffset(), v);
      break;
    case QMetaMethod::Method:
    case QMetaMethod::Slot:
      pC->MethodInvoke(pMO, id, v);
      break;
    default:
      break;
  }
}

// MOC function - retrieves the metaobject for an instance - used internally and externally
// This will lazily create the QMetaObject
const QMetaObject *QtEPComponent::metaObject() const
{
  if (!pMetaObj && pComponent)
  {
    pMetaObj = QtMetaObjectGenerator::Generate(pComponent->getDescriptor());
    EPASSERT_THROW(pMetaObj, ep::Result::Failure, "Unable to generate QMetaObject for component type '{0}'", pComponent->getType());
  }

  return pMetaObj;
}

// MOC function - used internally by qobject_cast
void *QtEPComponent::qt_metacast(const char *cname)
{
  if (!qstrcmp(cname, "QtTestComponent"))
    return (void*)this;
  return QObject::qt_metacast(cname);
}

// MOC function - instance based meta access goes via here
int QtEPComponent::qt_metacall(QMetaObject::Call call, int id, void **v)
{
  // See if this is a QObject meta call
  // Note that regular Qt behaviour is to try from the parent down, offsetting the id along the way
  // A negative id indicates the parent handled it
  int retId = QObject::qt_metacall(call, id, v);
  if (retId < 0)
    return retId;

  // This will spin up the meta object if we don't already have one
  const QMetaObject *pMO = metaObject();

  switch (call)
  {
    // Funnel meta method access
    case QMetaObject::InvokeMetaMethod:
    {
      // Walk up the meta stack to the appropriate object
      while (pMO->methodOffset() > id)
        pMO = pMO->superClass();

      switch (pMO->method(id).methodType())
      {
        case QMetaMethod::Signal:
          QMetaObject::activate(this, QMetaObjectPrivate::signalOffset(pMO), id - pMO->methodOffset(), v);
          retId -= pMO->methodCount();
          break;
        case QMetaMethod::Method:
        case QMetaMethod::Slot:
          retId = MethodInvoke(pMO, id, v);
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
    {
      // Walk up the meta stack to the appropriate object
      while (pMO->propertyOffset() > id)
        pMO = pMO->superClass();

      retId = PropertyInvoke(pMO, call, id, v);
      break;
    }
    // Don't do anything with queries but say we did
    case QMetaObject::QueryPropertyScriptable:
    case QMetaObject::QueryPropertyDesignable:
    case QMetaObject::QueryPropertyStored:
    case QMetaObject::QueryPropertyEditable:
    case QMetaObject::QueryPropertyUser:
      retId -= pMO->propertyCount();
      break;
    default:
      break;
  }

  EPASSERT_THROW(retId < 0, ep::Result::Failure, "Unsupported Qt Meta Access with index '{0}' and call type '{1}'", retId, (int)call);
  return retId;
}

// !! END INTERNAL MOC STUFF


// Internal
// Qt method access function
int QtEPComponent::MethodInvoke(const QMetaObject *pMO, int id, void **v)
{
  QMetaMethod method = pMO->method(id);

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

    QVariant retVal;
    String methodName = epFromQByteArray(method.name());

    // Check if this is a built-in method, otherwise pipe it thru to the EPComponent system
    if ((id - QObject::staticMetaObject.methodCount() - QtMetaObjectGenerator::builtInOffset) < QtMetaObjectGenerator::builtInCount)
      retVal = QtMetaObjectGenerator::builtInMethods[methodName](this, varArgs);
    else
      retVal = pComponent->call(methodName, (Slice<const Variant>)varArgs).as<QVariant>();

    if (method.returnType() != QMetaType::Void)
      *(QVariant*)v[0] = retVal;
  }
  catch (ep::EPException &) {}

  return id - pMO->methodCount();
}

// Internal
// Qt property access function
int QtEPComponent::PropertyInvoke(const QMetaObject *pMO, QMetaObject::Call call, int id, void **v)
{
  QMetaProperty prop = pMO->property(id);
  bool supported = true;

  try {
    switch (call)
    {
      case QMetaObject::ReadProperty:
        epFromVariant(pComponent->get(prop.name()), (QVariant*)*v);
        break;
      case QMetaObject::WriteProperty:
        pComponent->set(prop.name(), epToVariant(*(QVariant*)v[0]));
        break;
      default:
        supported = false;
        break;
    }
  }
  catch (ep::EPException &) {}

  EPASSERT_THROW(supported, ep::Result::Failure, "Unsupported meta property access of type '{0}'", (int)call);
  return id - pMO->propertyCount();
}

void QtEPComponent::connectNotify(const QMetaMethod &signal)
{
  ep::MutableString<0> signalName = epFromQByteArray(signal.name());

  // Check if we already have an entry in the connection map
  if (connectionMap.get(signalName))
    return;

  Connection *pConn = &connectionMap.insert(signalName, Connection{ this, signal });
  pConn->subscription = pComponent->subscribe(signalName, ep::VarDelegate(pConn, &QtEPComponent::Connection::SignalRouter));
}

void QtEPComponent::disconnectNotify(const QMetaMethod &signal)
{
  // If all signals have been disconnected, clean up everything
  if (!signal.isValid())
  {
    for (auto conn : connectionMap)
      conn.value.subscription->Unsubscribe();
    connectionMap.clear();
  }

  // If this was the last receiver for this signal, then clean up this connection
  if (receivers(QByteArray::number(QSIGNAL_CODE) + signal.methodSignature()) == 0)
  {
    auto conn = connectionMap.find(epFromQByteArray(signal.name()));
    conn->subscription->Unsubscribe();
    connectionMap.erase(conn);
  }
}

Variant QtEPComponent::Connection::SignalRouter(Slice<const Variant> args)
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
    QtEPComponent::qt_static_metacall(pComp, QMetaObject::InvokeMetaMethod, signal.methodIndex(), argv);
  return nullptr;
}


// Retrieves the window that the quick item currently belongs to
QQuickWindow *QtGlobalEPSingleton::parentWindow(QQuickItem *pQuickItem) const
{
  if (!pQuickItem)
  {
    QtApplication::kernel()->logError("Attempted to get parent window for null item");
    return nullptr;
  }
  return pQuickItem->window();
}

} // namespace qt

#else
EPEMPTYFILE
#endif
