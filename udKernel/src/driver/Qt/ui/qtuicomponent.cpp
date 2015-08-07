#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

// the qt implementation of the udui_driver requires a qt kernel currently
// error if we try otherwise
#if UDWINDOW_DRIVER != UDDRIVER_QT
#error UDUI_DRIVER Requires (UDWINDOW_DRIVER == UDDRIVER_QT)
#endif

#include <QQmlComponent>
//#include <QElapsedTimer>

#include "../udQtKernel_Internal.h"
#include "qtuicomponent.h"

namespace qt
{

// Instantiate QtComponent types
template class QtComponent<ud::UIComponent>;

// ---------------------------------------------------------------------------------------
template<typename Base>
const ud::PropertyInfo *QtComponent<Base>::GetPropertyInfo(udString property) const
{
  udDebugPrintf("QtComponent::GetPropertyInfo %s\n", property.ptr);
  // TODO: search for property, fall thru to component
  return Component::GetPropertyInfo(property);
}

// ---------------------------------------------------------------------------------------
//template<typename Base>
//ptrdiff_t QtComponent<Base>::PropertyIndex(udString property) const
//{
//  udDebugPrintf("QtComponent::PropertyIndex %s\n", property.ptr);
  // TODO: search for property, fall thru to component
//  return Component::PropertyIndex(property);
//}

// ---------------------------------------------------------------------------------------
template<typename Base>
void QtComponent<Base>::SetProperty(udString property, const udVariant &value)
{
  udDebugPrintf("QtComponent::SetProperty %s %s\n", property.ptr, value.asString().toStringz());

#if 0
  // check qobject list
  int propertyIndex;
  if ((propertyIndex = pQtQuickItem->metaObject()->indexOfProperty(property.ptr)) != -1)
  {
    // TODO: check writable/handle errors??
    pQtQuickItem->metaObject()->property(propertyIndex).write(pQtQuickItem, value.as<QVariant>());
  }
  // check dynamic property list
  else if (pQtQuickItem->dynamicPropertyNames().contains(property.ptr))
  {
    pQtQuickItem->setProperty(property.ptr, value.as<QVariant>());
  }
  // fall back to the parent
#endif

  // TODO: check efficiency - this is less code and less ugly than the above but we're doing a get and a set
  // TODO: should we expose dynamic property creation to our code as well? how do we set a new property?
  if (!pQtObject->property(property.ptr).isValid())
    ud::Component::SetProperty(property, value);

  pQtObject->setProperty(property.ptr, value.as<QVariant>());

  //propertyChange[pDesc->index].Signal();
}

// ---------------------------------------------------------------------------------------
template<typename Base>
udVariant QtComponent<Base>::GetProperty(udString property) const
{
  udDebugPrintf("QtComponent::GetProperty %s\n", property.ptr);
  QVariant propertyValue = pQtObject->property(property.ptr);

  // if we didn't find the property in qt fall thru to the ud components
  if (!propertyValue.isValid())
    return ud::Component::GetProperty(property);

  return udVariant(propertyValue);
}

// ---------------------------------------------------------------------------------------
template<typename Base>
void QtComponent<Base>::Init(udInitParams initParams)
{
  UDASSERT(pQtObject, "QObject must not be null");

  // TODO: optimise this - prevent redundant init path
  Component::Init(initParams);

  for (auto &kv : initParams)
  {
    // TODO: check constant?

    udString property = kv.key.asString();
    if (pQtObject->property(property.ptr).isValid())
      pQtObject->setProperty(property.ptr, kv.value.as<QVariant>());
  }

  // TODO: signal handler?
}


// ---------------------------------------------------------------------------------------
QtUIComponent::QtUIComponent(const ud::ComponentDesc *pType, ud::Kernel *pKernel, udRCString uid, udInitParams initParams)
  : QtComponent(pType, pKernel, uid, initParams)
  //, pDesc(pType)
{
  QString filename = initParams["file"].as<QString>();
  if (filename.isNull())
  {
    udDebugPrintf("Error: attempted to create ui component without qml file set\n");
    //udFree(pDesc);
    throw udR_Failure_;
  }

  // create the qml component for the associated script
  // TODO: remove hardcoded script
  QtKernel *pQtKernel = static_cast<QtKernel*>(pKernel);
  QQmlComponent component(pQtKernel->QmlEngine(), QUrl(filename));
  pQtObject = component.create();

  if (!pQtObject)
  {
    // TODO: better error information/handling
    udDebugPrintf("Error creating QtUIComponent\n");
    foreach(const QQmlError &error, component.errors())
      udDebugPrintf("QML ERROR: %s\n", error.toString().toLatin1().data());
    //udFree(pDesc);
    throw udR_Failure_;
  }

  // Try to cast to a QQuickItem
  // TODO: better error handling?
  pQtQuickItem = qobject_cast<QQuickItem*>(pQtObject);
  if (!pQtQuickItem)
  {
    //udFree(pDesc);
    throw udR_Failure_;
  }

  pQtQuickItem->setObjectName("TEST QT UI COMPONENT");

  //QElapsedTimer timer;
  //qint64 nanoSec;
  //timer.start();

  // Decorate the descriptor with meta object information
  //DecorateComponentDesc(pObject);

  //nanoSec = timer.nsecsElapsed();

  //qDebug() << "Decorating the component descriptor took " << nanoSec << "nanoseconds";

  // Build the descriptor search tree
  //pDesc->BuildSearchTree();
}

// ---------------------------------------------------------------------------------------
QtUIComponent::~QtUIComponent()
{
  delete pQtQuickItem;
  //udFree(const_cast<ud::EventDesc*>(pDesc->events.ptr));
  //udFree(const_cast<ud::MethodDesc*>(pDesc->methods.ptr));
  //udFree(const_cast<ud::PropertyDesc*>(pDesc->properties.ptr));
  //udFree(pDesc);
}

#if 0
// ---------------------------------------------------------------------------------------
template<typename Base>
ud::ComponentDesc *QtComponent<Base>::CreateComponentDesc(const ud::ComponentDesc *pType)
{
  ud::ComponentDesc *pCompDesc = udAllocType(ud::ComponentDesc, 1, udAF_Zero);

  // TODO: make an internal component lookup table if we end up needing multiple components for the one qml file

  pCompDesc->pSuperDesc = &ud::UIComponent::descriptor;
  pCompDesc->udVersion = ud::UDSHELL_APIVERSION;
  pCompDesc->pluginVersion = ud::UDSHELL_PLUGINVERSION;

  // TODO: should we use unique qt id's here?
  pCompDesc->id = ud::UIComponent::descriptor.id;
  pCompDesc->displayName = ud::UIComponent::descriptor.displayName;
  pCompDesc->description = ud::UIComponent::descriptor.description;

  return pCompDesc;
}
/*
struct QtGetter : public ud::Getter
{
public:
  QtGetter() : ud::Getter(nullptr) {}
  template <typename X, typename Type>
  QtGetter(Type(X::*func)(const char *name) const)
  {
    m = fastdelegate::MakeDelegate((X*)nullptr, func).GetMemento();
    shim = &shimFunc<Type>;
  }
};*/

// ---------------------------------------------------------------------------------------
void QtUIComponent::DecorateComponentDesc(QObject *pObject)
{
  const QMetaObject *pMetaObject = pObject->metaObject();

  // TODO: collapse these into one alloc - void *metaData;
  /*static char propertyDescStr[] = "Qt UI Component Property";
  static char methodDescStr[] = "Qt UI Component Method";

  // Fill in the property information
  ud::PropertyDesc *properties = udAllocType(ud::PropertyDesc, pMetaObject->propertyCount(), udAF_Zero);
  for (int i = 0; i < pMetaObject->propertyCount(); i++)
  {
    //qDebug() << "here";
    QMetaProperty property = pMetaObject->property(i);
    //properties[i].id = AllocUDStringFromQString(property.name());
    properties[i].id = udString(property.name());
    properties[i].displayName = properties[i].id;
    properties[i].description = propertyDescStr;

    //if (property.isReadable())
      //properties[i].getter = &QtUIComponent::Test;

    //pObject->property(properties[i].id);
  }

  // Fill in the method information
  ud::MethodDesc *methods = udAllocType(ud::MethodDesc, pMetaObject->methodCount(), udAF_Zero);
  //ud::EventDesc *events = udAllocType(ud::EventDesc, 1, udAF_Zero);
  for (int i = 0; i < pMetaObject->methodCount(); i++)
  {
    QMetaMethod method = pMetaObject->method(i);
    //methods[i].id = AllocUDStringFromQString(method.name());
    methods[i].id = udString(method.name());
    methods[i].displayName = methods[i].id;
    methods[i].description = methodDescStr;
  }*/

  //id -- name
  //displayName -- name
  //description -- qt ui component property
  //getter -- if isReadable() -- read
  //setter -- if isWritable() -- write
  //type -- type()
  //const_cast<ud::PropertyDesc *>(pDesc->properties.ptr) = properties;

  // Fill in the method information
  //ud::MethodDesc *methods = udAllocType(ud::MethodDesc, 1, udAF_Zero);

  //id -- name
  //displayName -- name
  //description -- qt ui component method
  //method --
  //result
  //args

  //const_cast<ud::MethodDesc *>(pDesc->methods.ptr) = methods;

  // Fill in the event information
  //ud::EventDesc *events = udAllocType(ud::EventDesc, 1, udAF_Zero);
  //const_cast<ud::EventDesc *>(pDesc->events.ptr) = events;

  qDebug() << "CLASS NAME: " << pMetaObject->className();

  ///// Property list - properties
  qDebug() << "NUMBER OF PROPERTIES: " << pMetaObject->propertyCount();

  for (int i = 0; i < pMetaObject->propertyCount(); i++)
  {
    qDebug() << "PROPERTY: " << pMetaObject->property(i).name() << " " << pMetaObject->property(i).typeName() << " " << pMetaObject->property(i).read(pObject);
  }

  qDebug() << "NUMBER OF DYNAMIC PROPERTIES: " << pObject->dynamicPropertyNames().size();
  foreach(const QByteArray &propName, pObject->dynamicPropertyNames())
  {
    qDebug() << "DYNAMIC PROPERTY: " << propName << " " << pObject->property(propName);
  }

  ///// Method list - slots
  qDebug() << "NUMBER OF METHODS: " << pMetaObject->methodCount();

  for (int i = 0; i < pMetaObject->methodCount(); i++)
  {
    qDebug() << "METHOD: " << pMetaObject->method(i).name() << " " << pMetaObject->method(i).methodSignature() << " " << pMetaObject->method(i).parameterCount() << " " << pMetaObject->method(i).methodType();
  }

  ///// Event list - signals
}
#endif

} // namespace qt

#endif  // UDUI_DRIVER == UDDRIVER_QT
