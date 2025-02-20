#include "driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "driver/qt/epkernel_qt.h"
#include "driver/qt/util/signaltodelegate_qt.h"

#include <QtCore/QByteArray>
#include <QtCore/QMetaType>


// !!Begin MOC generated code - any hand edits will be marked as !!HAND EDIT!!
// This voodoo allows us to hook into the Qt Meta System and redirect all signals via the execute() method
// NOTE! This was generated using MOC for Qt 5.6; it's possible that this could break when compiling against a future version of Qt
// In these instances we will need to regenerate the MOC file and port the hand edit code

struct qt_meta_stringdata_qt__QtSignalMapper_t {
  QByteArrayData data[5];
  char stringdata0[53];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
  Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
  qptrdiff(offsetof(qt_meta_stringdata_qt__QtSignalMapper_t, stringdata0) + ofs \
    - idx * sizeof(QByteArrayData)) \
  )
static const qt_meta_stringdata_qt__QtSignalMapper_t qt_meta_stringdata_qt__QtSignalMapper = {
  {
    QT_MOC_LITERAL(0, 0, 18), // "qt::QtSignalMapper"
    QT_MOC_LITERAL(1, 19, 7), // "execute"
    QT_MOC_LITERAL(2, 27, 0), // ""
    QT_MOC_LITERAL(3, 28, 19), // "onInstanceDestroyed"
    QT_MOC_LITERAL(4, 48, 4) // "pObj"

  },
  "qt::QtSignalMapper\0execute\0\0"
  "onInstanceDestroyed\0pObj"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_qt__QtSignalMapper[] = {
// content:
    7,       // revision
    0,       // classname
    0,    0, // classinfo
    2,   14, // methods
    0,    0, // properties
    0,    0, // enums/sets
    0,    0, // constructors
    0,       // flags
    0,       // signalCount

// slots: name, argc, parameters, tag, flags
    1,    0,   24,    2, 0x08 /* Private */,
    3,    1,   25,    2, 0x08 /* Private */,

// slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QObjectStar,    4,

    0        // eod
};


void qt::QtSignalMapper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
  if (_c == QMetaObject::InvokeMetaMethod) {
    QtSignalMapper *_t = static_cast<QtSignalMapper *>(_o);
    Q_UNUSED(_t)
    switch (_id) {
      case 0: _t->execute(_a); break; // !!HAND EDIT!! - Add the _a parameter
      case 1: _t->onInstanceDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
      default:;
    }
  }
}

const QMetaObject qt::QtSignalMapper::staticMetaObject = {
  { &QObject::staticMetaObject, qt_meta_stringdata_qt__QtSignalMapper.data,
    qt_meta_data_qt__QtSignalMapper,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR }
};


const QMetaObject *qt::QtSignalMapper::metaObject() const
{
  return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *qt::QtSignalMapper::qt_metacast(const char *_clname)
{
  if (!_clname) return Q_NULLPTR;
  if (!strcmp(_clname, qt_meta_stringdata_qt__QtSignalMapper.stringdata0))
    return static_cast<void*>(const_cast< QtSignalMapper*>(this));
  return QObject::qt_metacast(_clname);
}

int qt::QtSignalMapper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
  _id = QObject::qt_metacall(_c, _id, _a);
  if (_id < 0)
    return _id;
  if (_c == QMetaObject::InvokeMetaMethod) {
    if (_id < 2)
      qt_static_metacall(this, _c, _id, _a);
    _id -= 2;
  }
  else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
    if (_id < 2)
      *reinterpret_cast<int*>(_a[0]) = -1;
    _id -= 2;
  }
  return _id;
}

// !!End MOC generated code

using namespace ep;

namespace qt {

SubscriptionRef QtSignalMapper::Subscribe(QObject *pSourceObj, const ep::VarDelegate &del)
{
  QtConnection *pConnection = instanceMap.get(pSourceObj);

  if (!pConnection)
  {
    pConnection = &(instanceMap.insert(pSourceObj, QtConnection()));
    QObject::connect(pSourceObj, &QObject::destroyed, this, &QtSignalMapper::onInstanceDestroyed);

    pConnection->connection = QObject::connect(pSourceObj, pSourceObj->metaObject()->method(signalId), this, metaObject()->method(metaObject()->methodOffset()));
    EPTHROW_IF(!pConnection->connection, Result::Failure, "Signal Mapper connection failed: Cannot map signal '{0}'", signalName);
  }

  return pConnection->event.addSubscription(del);
}

void QtSignalMapper::execute(void **args)
{
  // Find the calling instance in the map
  QObject *pSender = sender();
  EPASSERT_THROW(pSender, Result::Failure, "Could not determine the calling instance of Qt Event {0}", signalName);
  QtConnection *pConnection = instanceMap.get(pSender);
  EPASSERT_THROW(pConnection, Result::Failure, "Could not locate the calling instance in the Signal Mapper for Qt Event {0}", signalName);

  // If there's no more active subscribers for this instance, let's unregister
  if (!pConnection->event.hasSubscribers())
  {
    instanceMap.remove(pSender);
    return;
  }

  QMetaMethod signal = pSender->metaObject()->method(signalId);

  // Loop thru the parameters for the mapped signal and convert to ep::Variants
  Array<Variant, Q_METAMETHOD_INVOKE_MAX_ARGS> varArgs(ep::Reserve, signal.parameterCount());
  for (int i = 0; i < signal.parameterCount(); ++i)
  {
    // Note that args[0] is the return value
    // We initialise a QVariant based on the parameter type - if the param is already a QVariant then pass it direct
    int type = signal.parameterType(i);
    if (type == QMetaType::QVariant)
      varArgs.pushBack(epToVariant(*(QVariant*)args[i + 1]));
    else
      varArgs.pushBack(epToVariant(QVariant(type, args[i + 1])));
  }
  pConnection->event.signal(varArgs);
}

} // namespace qt

#else
EPEMPTYFILE
#endif  // EPUI_DRIVER == EPDRIVER_QT
