#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

// the qt implementation of the udui_driver requires a qt kernel currently
// error if we try otherwise
#if UDWINDOW_DRIVER != UDDRIVER_QT
#error UDUI_DRIVER Requires (UDWINDOW_DRIVER == UDDRIVER_QT)
#endif

#include <QQmlComponent>

#include "../udQtKernel_Internal.h"
#include "qtuicomponent.h"

namespace qt
{

// Instantiate QtComponent types
template class QtComponent<ud::UIComponent>;

// Qt shims
struct QtGetter : public ud::Getter
{
public:
  QtGetter(nullptr_t) : ud::Getter(nullptr) {}
  template<typename QtComponentT>
  QtGetter(udString id, QtComponentT *arg) : Getter(nullptr), propertyId(id)
  {
    shim = &shimFunc<QtComponentT>;
  }

protected:
  udString propertyId;

  template<typename QtComponentT>
  static udVariant shimFunc(const Getter * const _pGetter, const ud::Component *pThis)
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
  QtSetter(udString id, QtComponentT *arg) : Setter(nullptr), propertyId(id)
  {
    shim = &shimFunc<QtComponentT>;
  }

protected:
  udString propertyId;

  template<typename QtComponentT>
  static void shimFunc(const Setter * const _pSetter, ud::Component *pThis, const udVariant &value)
  {
    QtSetter *pSetter = (QtSetter*)_pSetter;
    QtComponentT *pQtComp = (QtComponentT *)pThis;
    pQtComp->QtObject()->setProperty(pSetter->propertyId.ptr, value.as<QVariant>());
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

  static char displayName[] = "Qt UIComponent";
  static char description[] = "Is a Qt UI component";

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

  const QMetaObject *pMetaObject = pObject->metaObject();

  // Inject the properties
  for (int i = 0; i < pMetaObject->propertyCount(); ++i)
  {
    // TODO: move this to class
    static char propertyDescStr[] = "Qt UI Component Property";

    QMetaProperty property = pMetaObject->property(i);

    // TODO: keep a list of string names that we manage so we can free
    // TODO: type & flags
    udString propertyName = AllocUDStringFromQString(property.name());
    ud::PropertyInfo info = { propertyName, propertyName, propertyDescStr };

    // TODO: store list to free getter/setter?
    ud::PropertyDesc desc = { info, udNew(QtGetter, propertyName, this), udNew(QtSetter, propertyName, this) };

    pDesc->propertyTree.Insert(propertyName, desc);
  }
}


// ---------------------------------------------------------------------------------------
QtUIComponent::QtUIComponent(ud::ComponentDesc *pType, ud::Kernel *pKernel, udRCString uid, udInitParams initParams)
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

} // namespace qt

#endif  // UDUI_DRIVER == UDDRIVER_QT
