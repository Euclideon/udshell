#pragma once
#ifndef QMLBINDINGS_QT_H
#define QMLBINDINGS_QT_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/component/uicomponent.h"
#include "ep/cpp/hashmap.h"
#include "driver/qt/epkernel_qt.h"

#include <QQuickItem>
#include <QQuickWindow>

namespace qt {

// forward declare
class QtFocusManager;
class QtEPComponent;
struct QtEPMetaObject;

namespace internal {

// ---------------------------------------------------------------------------------------
// INTERNAL

// Helper function to dynamically populate the component descriptor with the QObject's meta data
void PopulateComponentDesc(ep::ComponentDescInl *pDesc, QObject *pObject);

} // namespace internal


// Helper class that gets stored with the Dynamic Component Descriptor of Qt based components
// This is responsible for loading the QML file, and creating both the associated dynamic component and qobject instance
class QmlComponentData : public ep::RefCounted
{
public:
  QmlComponentData(ep::String file, QQmlEngine *pQmlEngine, QQmlComponent::CompilationMode mode = QQmlComponent::PreferSynchronous)
    : qmlComponent(pQmlEngine)
  {
    EPASSERT_THROW(!file.empty(), ep::Result::InvalidArgument, "Must supply file name");

    QString fileStr = QString::fromUtf8(file.ptr, (int)file.length);
    if (file.ptr[0] == ':')
      fileStr.prepend(QStringLiteral("qrc"));

    // create the qml component for the associated script
    qmlComponent.loadUrl(QUrl(fileStr), mode);
  }

  ep::DynamicComponentRef CreateComponent(ep::KernelRef spKernel, ep::Variant::VarMap initParams = nullptr);
  QObject *createInstance(QQmlEngine *pQmlEngine, ep::Component *pGlueComponent, ep::Variant::VarMap initParams);

private:
  QQmlComponent qmlComponent;
};


// Helper factory class which creates a new QMetaObject based on a EP Component Descriptor
class QtMetaObjectGenerator
{
public:
  static const QtEPMetaObject *Generate(const ep::ComponentDesc *pDesc);
  static void ClearCache();

private:
  friend class QtEPComponent;
  using RunBuiltInFunc = std::function<QVariant(QtEPComponent *pObj, ep::Slice<const ep::Variant> varArgs)>;

  const char * const pArgNames[10] = { "arg0", "arg1", "arg2", "arg3", "arg4", "arg5", "arg6", "arg7", "arg8", "arg9" };
  static_assert(Q_METAMETHOD_INVOKE_MAX_ARGS == 10, "Qt's MetaMethod max args size has changed");

  enum class MethodType
  {
    Signal,
    Slot
  };

  struct Property
  {
    QByteArray name;
    uint flags;
  };

  struct Method
  {
    QByteArray name;
    ep::SharedArray<uint> paramTypes;
    uint flags;
    uint returnType;
  };

  QtMetaObjectGenerator(const ep::ComponentDesc *pDesc);
  ~QtMetaObjectGenerator() {}

  QtEPMetaObject *CreateMetaObject(const QMetaObject *pParent);

  void AddProperty(const ep::PropertyDesc *pProperty);
  void AddMethod(MethodType type, ep::String name, ep::Slice<const ep::SharedString> params = nullptr, RunBuiltInFunc runMethod = nullptr, int numDefaultArgs = 0);

  void AddSlot(const ep::MethodDesc *pMethod) {
    // TODO: Detect if this method is from a scripting language and needs to be vararg OR has default params
    AddMethod(MethodType::Slot, pMethod->id, pMethod->argTypes);
  }
  void AddSignal(const ep::EventDesc *pEvent) {
    // TODO: Detect if this method is from a scripting language and needs to be vararg OR has default params
    AddMethod(MethodType::Signal, pEvent->id, pEvent->argTypes);
  }

  int AggregateParamCount(ep::Slice<Method> methodList);

  static ep::HashMap<ep::SharedString, QtEPMetaObject*> metaObjectCache;
  static ep::HashMap<ep::SharedString, RunBuiltInFunc> builtInMethods;
  static int builtInOffset;
  static int builtInCount;

  ep::Array<Property> propertyList;
  ep::Array<Method> slotList;
  ep::Array<Method> signalList;

  const ep::ComponentDescInl *pDesc;
};


// ---------------------------------------------------------------------------------------
// SHIM OBJECTS

struct BuildQtEPComponent;
enum QtHasWeakRef_t { QtHasWeakRef };

// This shim class wraps an ep::Component in a QObject that can be accessible from QML
// A special QMetaObject will be generated in order to reflect the EP Meta Data and expose it to Qt
class QtEPComponent : public QObject
{
  Q_OBJECT_FAKE

public:
  ~QtEPComponent() {}

  ep::ComponentRef GetComponent() const { return ep::ComponentRef(pComponent); }

private:
  friend class QtMetaObjectGenerator;
  friend struct BuildQtEPComponent;

  struct Connection
  {
    QtEPComponent *pComp;
    QMetaMethod signal;
    ep::SubscriptionRef subscription;

    ep::Variant SignalRouter(ep::Slice<const ep::Variant>);
  };

  QtEPComponent() : QObject(nullptr), pComponent(nullptr) {}
  QtEPComponent(const ep::ComponentRef &spComponent) : QObject(nullptr), spComponent(spComponent) { pComponent = spComponent.ptr(); }
  QtEPComponent(ep::ComponentRef &&rval) : QObject(nullptr), spComponent(std::move(rval)) { pComponent = spComponent.ptr(); }
  QtEPComponent(const QtEPComponent &val)
    : QObject(val.parent()), pMetaObj(val.pMetaObj), spComponent(ep::ComponentRef(val.pComponent)), pComponent(val.pComponent)
  {
  }

  // this constructor ensures the QtEPComponent only holds a weak pointer to its ep::Component
  // currently only used to define the QML "thisComponent" since a SharedPtr will result in a circular reference
  QtEPComponent(ep::Component *pComp, QtHasWeakRef_t) : QObject(nullptr), pComponent(pComp) {}

  // Internal Meta Magic functions
  int MethodInvoke(const QMetaObject *pMO, int id, void **v);
  int PropertyInvoke(const QMetaObject *pMO, QMetaObject::Call call, int id, void **v);

  void connectNotify(const QMetaMethod &signal) override;
  void disconnectNotify(const QMetaMethod &signal) override;

  mutable const QMetaObject *pMetaObj = nullptr;
  ep::ComponentRef spComponent;
  ep::Component *pComponent = nullptr;    // used to avoid circular references

  ep::HashMap<ep::SharedString, Connection> connectionMap;
};


// This shim class wraps our qt::QtKernel specialised kernel in a QObject that can be accessible from QML
class QtKernelQml : public QObject
{
  Q_OBJECT

public:
  QtKernelQml(QtKernel *_pKernel, QObject *pParent = nullptr) : QObject(pParent), pKernel(_pKernel) {}

  Q_PROPERTY(QtFocusManager * focus READ getFocusManager)

  Q_INVOKABLE qt::QtEPComponent *findComponent(const QString &uid) const;
  Q_INVOKABLE qt::QtEPComponent *createComponent(const QString typeId, QVariant initParams = QVariant());
  Q_INVOKABLE qt::QtEPComponent *getCommandManager() const;

  Q_INVOKABLE qt::QtEPComponent *getLua() const;
  Q_INVOKABLE qt::QtEPComponent *getStdOutBroadcaster() const;
  Q_INVOKABLE qt::QtEPComponent *getStdErrBroadcaster() const;
  Q_INVOKABLE void exec(QString str);

private:
  QtFocusManager *getFocusManager() const;

  QtKernel *pKernel;
};

// This class provides some helper functions that are accessible from QML via the EP global singleton
class QtGlobalEPSingleton : public QObject
{
  Q_OBJECT

public:
  QtGlobalEPSingleton(QObject *pParent = nullptr) : QObject(pParent) {}

  Q_INVOKABLE QQuickWindow *parentWindow(QQuickItem *pQuickItem) const;
};


// factory class to build a qml shim component from a component
struct BuildQtEPComponent
{
  // create a qml shim component that has a shared ref to the ep::Component
  static QtEPComponent *Create(const ep::ComponentRef &spComponent) { return (spComponent ? new QtEPComponent(spComponent) : nullptr); }
  static QtEPComponent *Create(ep::ComponentRef &&spComponent) { return (spComponent ? new QtEPComponent(std::move(spComponent)) : nullptr); }

  // create a qml shim component that has a weak ref to the ep::component - used to avoid circular deps
  static QtEPComponent *CreateWeak(ep::Component *pThis) { return (pThis ? new QtEPComponent(pThis, QtHasWeakRef) : nullptr); }
};

} // namespace qt

// This ensures qobject_cast works with our fake QtTestComponent objects
template <> inline qt::QtEPComponent *qobject_cast<qt::QtEPComponent*>(const QObject *o)
{
  void *result = o ? const_cast<QObject *>(o)->qt_metacast("QtTestComponent") : 0;
  return (qt::QtEPComponent*)(result);
}
template <> inline qt::QtEPComponent *qobject_cast<qt::QtEPComponent*>(QObject *o)
{
  void *result = o ? o->qt_metacast("QtTestComponent") : 0;
  return (qt::QtEPComponent*)(result);
}

#endif  // QMLBINDINGS_QT_H
