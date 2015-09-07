#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

// the qt implementation of the udui_driver requires a qt kernel currently
// error if we try otherwise
#if UDWINDOW_DRIVER != UDDRIVER_QT
#error UDUI_DRIVER Requires (UDWINDOW_DRIVER == UDDRIVER_QT)
#endif

#include <QQmlComponent>

#include "../udQtKernel_Internal.h"
#include "../util/signaltodelegate.h"
#include "qtuicomponent.h"


namespace qt
{

// Qt shims
struct QtGetter : public ud::Getter
{
public:
  QtGetter(nullptr_t) : ud::Getter(nullptr) {}
  template<typename QtComponentT>
  QtGetter(udString id, QtComponentT *arg) : ud::Getter(nullptr), propertyId(id)
  {
    shim = &shimFunc<QtComponentT>;
  }

protected:
  udString propertyId;

  template<typename QtComponentT>
  static udVariant shimFunc(const ud::Getter * const _pGetter, const ud::Component *pThis)
  {
    QtGetter *pGetter = (QtGetter*)_pGetter;
    const QtComponentT *pQtComp = (const QtComponentT *)pThis;
    return udVariant(pQtComp->QtObject()->property(pGetter->propertyId.ptr));
  }
};


struct QtSetter : public ud::Setter
{
public:
  QtSetter(nullptr_t) : ud::Setter(nullptr) {}
  template<typename QtComponentT>
  QtSetter(udString id, QtComponentT *arg) : ud::Setter(nullptr), propertyId(id)
  {
    shim = &shimFunc<QtComponentT>;
  }

protected:
  udString propertyId;

  template<typename QtComponentT>
  static void shimFunc(const ud::Setter * const _pSetter, ud::Component *pThis, const udVariant &value)
  {
    QtSetter *pSetter = (QtSetter*)_pSetter;
    QtComponentT *pQtComp = (QtComponentT *)pThis;
    pQtComp->QtObject()->setProperty(pSetter->propertyId.ptr, value.as<QVariant>());
  }
};


struct QtMethod : public ud::Method
{
public:
  QtMethod(nullptr_t) : ud::Method(nullptr) {}
  template<typename QtComponentT>
  QtMethod(const QMetaMethod &_method, QtComponentT *arg) : ud::Method(nullptr), method(_method)
  {
    shim = &shimFunc<QtComponentT>;
  }

protected:
  QMetaMethod method;

  template<typename QtComponentT>
  static udVariant shimFunc(const Method * const _pMethod, ud::Component *pThis, udSlice<udVariant> value)
  {
    QtMethod *pMethod = (QtMethod*)_pMethod;
    QtComponentT *pQtComp = (QtComponentT *)pThis;

    // TODO: do better runtime handling of this rather than assert since this can come from the user - check against Q_METAMETHOD_INVOKE_MAX_ARGS
    // TODO: error output??
    UDASSERT(value.length <= 10, "Attempting to call method shim with more than 10 arguments");

    // TODO: Do something less ugly - list of QGenericArguments?
    // TODO: check value length against function arg length minus default args amount - need to parse the signature to get the default arg list?
    QVariant retVal;
    switch (value.length)
    {
      case 0:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal));
        break;
      case 1:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()));
        break;
      case 2:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()));
        break;
      case 3:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()), Q_ARG(QVariant, value[2].as<QVariant>()));
        break;
      case 4:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()), Q_ARG(QVariant, value[2].as<QVariant>()),
          Q_ARG(QVariant, value[3].as<QVariant>()));
        break;
      case 5:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()), Q_ARG(QVariant, value[2].as<QVariant>()),
          Q_ARG(QVariant, value[3].as<QVariant>()), Q_ARG(QVariant, value[4].as<QVariant>()));
        break;
      case 6:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()), Q_ARG(QVariant, value[2].as<QVariant>()),
          Q_ARG(QVariant, value[3].as<QVariant>()), Q_ARG(QVariant, value[4].as<QVariant>()), Q_ARG(QVariant, value[5].as<QVariant>()));
        break;
      case 7:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()), Q_ARG(QVariant, value[2].as<QVariant>()),
          Q_ARG(QVariant, value[3].as<QVariant>()), Q_ARG(QVariant, value[4].as<QVariant>()), Q_ARG(QVariant, value[5].as<QVariant>()),
          Q_ARG(QVariant, value[6].as<QVariant>()));
        break;
      case 8:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()), Q_ARG(QVariant, value[2].as<QVariant>()),
          Q_ARG(QVariant, value[3].as<QVariant>()), Q_ARG(QVariant, value[4].as<QVariant>()), Q_ARG(QVariant, value[5].as<QVariant>()),
          Q_ARG(QVariant, value[6].as<QVariant>()), Q_ARG(QVariant, value[7].as<QVariant>()));
        break;
      case 9:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()), Q_ARG(QVariant, value[2].as<QVariant>()),
          Q_ARG(QVariant, value[3].as<QVariant>()), Q_ARG(QVariant, value[4].as<QVariant>()), Q_ARG(QVariant, value[5].as<QVariant>()),
          Q_ARG(QVariant, value[6].as<QVariant>()), Q_ARG(QVariant, value[7].as<QVariant>()), Q_ARG(QVariant, value[8].as<QVariant>()));
        break;
      case 10:
      default:
        pMethod->method.invoke(pQtComp->QtObject(), Qt::AutoConnection, Q_RETURN_ARG(QVariant, retVal),
          Q_ARG(QVariant, value[0].as<QVariant>()), Q_ARG(QVariant, value[1].as<QVariant>()), Q_ARG(QVariant, value[2].as<QVariant>()),
          Q_ARG(QVariant, value[3].as<QVariant>()), Q_ARG(QVariant, value[4].as<QVariant>()), Q_ARG(QVariant, value[5].as<QVariant>()),
          Q_ARG(QVariant, value[6].as<QVariant>()), Q_ARG(QVariant, value[7].as<QVariant>()), Q_ARG(QVariant, value[8].as<QVariant>()),
          Q_ARG(QVariant, value[9].as<QVariant>()));
    }

    return udVariant(retVal);
  }
};


struct QtVarEvent : public ud::VarEvent
{
public:
  QtVarEvent(nullptr_t) : ud::VarEvent(nullptr) {}
  template<typename QtComponentT>
  QtVarEvent(const QMetaMethod &m, QtComponentT *arg) : ud::VarEvent(nullptr), method(m), sigToDel(nullptr)
  {
    pSubscribe = &doSubscribe<QtComponentT>;
  }

protected:
  QMetaMethod method;
  QtSignalToDelegate *sigToDel;

  template<typename QtComponentT>
  static void doSubscribe(const VarEvent *pEv, const ud::ComponentRef &c, const udVariant::VarDelegate &d)
  {
    QtVarEvent *pEvent = (QtVarEvent*)pEv;
    QtComponentT *pQtComp = (QtComponentT *)c.ptr();

    // TODO: hook up disconnect path
    // TODO: store list of signal to delegates
    pEvent->sigToDel = new QtSignalToDelegate(pQtComp->QtObject(), pEvent->method, d);
  }
};


// ---------------------------------------------------------------------------------------
template<typename Base>
QtComponent<Base>::~QtComponent()
{
  udFree(pDesc);
  delete pQtObject;
}

// ---------------------------------------------------------------------------------------
template<typename Base>
ud::ComponentDesc *QtComponent<Base>::CreateComponentDesc(const ud::ComponentDesc *pType)
{
  ud::ComponentDesc *pCompDesc = udAllocType(ud::ComponentDesc, 1, udAF_Zero);

  // TODO: override this in specialised derived class - "is a qt ui component"
  static char displayName[] = "Qt Component";
  static char description[] = "Is a Qt component";

  // TODO: make an internal component lookup table if we end up needing multiple components for the one qml file

  pCompDesc->pSuperDesc = const_cast<ud::ComponentDesc*>(pType);
  pCompDesc->udVersion = ud::UDSHELL_APIVERSION;
  pCompDesc->pluginVersion = ud::UDSHELL_PLUGINVERSION;

  // TODO: should we use unique qt id's here?
  pCompDesc->id = pType->id;
  pCompDesc->displayName = displayName;
  pCompDesc->description = description;

  return pCompDesc;
}

// ---------------------------------------------------------------------------------------
template<typename Base>
void QtComponent<Base>::PopulateComponentDesc(QObject *pObject)
{
  UDASSERT(pDesc, "ComponentDesc is null");

  // Build the descriptor search tree
  pDesc->BuildSearchTrees();

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
    ud::PropertyDesc desc = { info, udNew(QtGetter, propertyName, this), udNew(QtSetter, propertyName, this) };

    pDesc->propertyTree.Insert(propertyName, desc);
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
      ud::MethodDesc desc = { info, udNew(QtMethod, method, this) };

      pDesc->methodTree.Insert(methodName, desc);
    }
    else if (method.methodType() == QMetaMethod::Signal)
    {
      // TODO: move this to class
      static char eventDescStr[] = "Qt Component Event";

      // TODO: keep list of strings
      // TODO: keep free list of events
      udString eventName = AllocUDStringFromQString(method.name());
      ud::EventInfo info = { eventName, eventName, eventDescStr };
      ud::EventDesc desc = { info, udNew(QtVarEvent, method, this) };

      pDesc->eventTree.Insert(eventName, desc);
    }
  }
}


// ---------------------------------------------------------------------------------------
QtUIComponent::QtUIComponent(ud::ComponentDesc *pType, ud::Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : QtComponent(pType, pKernel, uid, initParams)
{
  QString filename = initParams["file"].as<QString>();
  if (filename.isNull())
  {
    udDebugPrintf("Error: attempted to create ui component without qml file set\n");
    udFree(pDesc);
    throw udR_Failure_;
  }

  // create the qml component for the associated script
  QtKernel *pQtKernel = static_cast<QtKernel*>(pKernel);
  QQmlComponent component(pQtKernel->QmlEngine(), QUrl(filename));
  pQtObject = component.create();

  if (!pQtObject)
  {
    // TODO: better error information/handling
    udDebugPrintf("Error creating QtUIComponent\n");
    foreach(const QQmlError &error, component.errors())
      udDebugPrintf("QML ERROR: %s\n", error.toString().toLatin1().data());
    udFree(pDesc);
    throw udR_Failure_;
  }

  // We expect a QQuickItem object
  // TODO: better error handling?
  UDASSERT(qobject_cast<QQuickItem*>(pQtObject) != nullptr, "QtUIComponents must create a QQuickItem");

  // Decorate the descriptor with meta object information
  PopulateComponentDesc(pQtObject);
}

// ---------------------------------------------------------------------------------------
QtUIComponent::~QtUIComponent()
{
}


// Instantiate QtComponent types
template class QtComponent<ud::UIComponent>;

} // namespace qt

#endif  // UDUI_DRIVER == UDDRIVER_QT
