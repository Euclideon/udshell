#include "driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "typeconvert_qt.h"

#include "driver/qt/components/qobjectcomponent_qt.h"
#include "driver/qt/epkernel_qt.h"
#include "driver/qt/util/qmlbindings_qt.h"

#include <QJSValueIterator>
#include <QMetaType>


// Qt type conversion to/from EP

using ep::Variant;
using ep::SharedString;
using ep::Slice;
using ep::Array;
using ep::shared_pointer_cast;


Variant epToVariant(QObject *pQObj)
{
  qt::QtEPComponent *pQC = qobject_cast<qt::QtEPComponent*>(pQObj);
  if (pQC)
    return Variant(pQC->GetComponent());

  epDebugPrintf("TODO: epToVariant: Unsupported QObject conversion '%s'; we need a global pKernel pointer >_<\n", pQObj->metaObject()->className());

  // TODO: create generic QObjectComponent which thinly wraps a QObject
//  pKernel->createComponent<QObjectComponent>({ { "object" }, { (int64_t)(size_t)pQObj } });
  return Variant(nullptr);
}

Variant epToVariant(const QVariant &var)
{
  if (!var.isValid())
    return Variant();
  if (var.isNull())
    return Variant(nullptr);

  switch (static_cast<QMetaType::Type>(var.type()))
  {
    // Variant::Type::Void
    case QMetaType::Void:
      return Variant();

    // Variant::Type::Bool
    case QMetaType::Bool:
      return Variant(var.toBool());

    // Variant::Type::Float
    case QMetaType::Float:
    case QMetaType::Double:
      return Variant(var.toDouble());

    // Variant::Type::Int
    case QMetaType::Char:
    case QMetaType::UChar:
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Long:
    case QMetaType::ULong:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
      return Variant((int64_t)var.toLongLong());

    // Variant::Type::String
    // TODO: optimise - reduce unnecessary copies
    case QMetaType::QByteArray:
    case QMetaType::QString:
    case QMetaType::QChar:
      return Variant(qt::epFromQString(var.toString()));

    case QMetaType::QObjectStar:
      return epToVariant(var.value<QObject*>());

    case QMetaType::VoidStar:
    {
      void *pPtr = var.value<void*>();
      if (pPtr == nullptr)
        return Variant(nullptr);
      EPASSERT(false, "TODO: handle raw buffers?");
      return Variant();
    }

    // if we have a variant in a variant, we need to unpeel
    case QMetaType::QVariant:
      return epToVariant(var.value<QVariant>());

    case QMetaType::User:
    {
      int userType = var.userType();
      if (userType == qMetaTypeId<QJSValue>())
        return epToVariant(var.value<QJSValue>());
    }

    default:
      epDebugPrintf("epToVariant: Unsupported type '%s' (support me!)\n", var.typeName());
      return Variant();
  }
}

void epFromVariant(const Variant &variant, QVariant *pVariant)
{
  EPASSERT(pVariant->isNull(), "pVariant is not null");

  switch (variant.type())
  {
    case Variant::Type::Void:
      pVariant->setValue(QVariant(QVariant::Invalid));
      break;

    case Variant::Type::Null:
      // *pVariant is already null right?
      break;

    case Variant::Type::Bool:
      pVariant->setValue(variant.asBool());
      break;

    case Variant::Type::Int:
      pVariant->setValue(variant.asInt());
      break;

    case Variant::Type::Float:
      pVariant->setValue(variant.asFloat());
      break;

    case Variant::Type::SharedPtr:
      switch (variant.spType())
      {
        case Variant::SharedPtrType::Component:
        {
          ep::ComponentRef spComponent = variant.asComponent();
          if (spComponent->isType("ep.QObjectComponent"))
            pVariant->setValue(shared_pointer_cast<qt::QObjectComponent>(spComponent)->getQObject());
          else
          {
            QObject *pQObject = qt::BuildQtEPComponent::Create(std::move(spComponent));
            pVariant->setValue(pQObject);

            // since we allocated this QObject, we'll set it to QML ownership so it can be freed
            QQmlEngine::setObjectOwnership(pQObject, QQmlEngine::JavaScriptOwnership);
          }
          break;
        }

        case Variant::SharedPtrType::Delegate:
        {
          using namespace qt;
          QtDelegate *del = new QtDelegate(variant.asDelegate());

          QJSValue jsDel = QtApplication::kernel()->qmlEngine()->newQObject(del);
          if (jsDel.hasProperty("call"))
            pVariant->setValue(jsDel.property("call"));

          QQmlEngine::setObjectOwnership(del, QQmlEngine::JavaScriptOwnership);
          break;
        }

        case Variant::SharedPtrType::Subscription:
        {
          using namespace qt;
          QtSubscription *pSub = new QtSubscription(variant.asSubscription());
          pVariant->setValue(pSub);
          QQmlEngine::setObjectOwnership(pSub, QQmlEngine::JavaScriptOwnership);
          break;
        }

        case Variant::SharedPtrType::AssocArray:
        {
          QJSValue jsMap = qt::QtApplication::kernel()->qmlEngine()->newObject();
          Variant::VarMap aa = variant.asAssocArray();
          for (auto v : aa)
          {
            if (!v.key.is(Variant::Type::String) && !v.key.is(Variant::Type::Int))
            {
              epDebugPrintf("epFromVariant: Key is not string!\n");
              continue;
            }
            QJSValue value;
            QString key;
            epFromVariant(v.key, &key);
            epFromVariant(v.value, &value);
            jsMap.setProperty(key, value);
          }
          pVariant->setValue(jsMap);
          break;
        }

        default:
          epDebugPrintf("epFromVariant: Unsupported SharedPtr type '%d'\n", (int)variant.spType());
          break;
      }
      break;

    // TODO: optimise?
    case Variant::Type::String:
      pVariant->setValue(variant.as<QString>());
      break;

    case Variant::Type::Array:
    {
      uint length = (uint)variant.arrayLen();
      QJSValue jsArray = qt::QtApplication::kernel()->qmlEngine()->newArray(length);
      Slice<Variant> varr = variant.asArray();
      for (uint i = 0; i < length; ++i)
      {
        QJSValue t;
        epFromVariant(varr[i], &t);
        jsArray.setProperty(i, t);
      }
      pVariant->setValue(jsArray);
      break;
    }

    default:
      epDebugPrintf("epFromVariant: Unsupported type '%d'\n", (int)variant.type());
  }
}

Variant epToVariant(const QJSValue &v)
{
  // TODO: investigate if we can switch on some numeric type id?

  if (v.isNumber())
    return Variant(v.toNumber());
  else if (v.isString())
    return Variant(qt::epFromQString(v.toString()));
  else if (v.isBool())
    return Variant(v.toBool());
  else if (v.isNull())
    return Variant(nullptr);
  else if (v.isVariant())
    return epToVariant(v.toVariant());
  else if (v.isQObject())
    return epToVariant(v.toQObject());
  else if (v.isCallable())
    return ep::VarDelegate(qt::JSValueDelegateRef::create(v));
  else if (v.isArray())
  {
    size_t length = (size_t)v.property(QString("length")).toNumber();
    Array<Variant> r(ep::Reserve, length);

    QJSValueIterator i(v);
    size_t index = 0;
    while (i.hasNext() && index < length)
    {
      i.next();
      EPASSERT(i.name() == QString::number(index++), "Array has fail sequence");
      r.pushBack(epToVariant(i.value()));
    }
    return Variant(std::move(r));
  }
  else if (v.isError())
  {
    // TODO: translate error info to errorstate?
    QString errorStr = QString("Javascript Exception: %0 (%1:%2) - %3\nStack: %4")
      .arg(v.property("name").toString())
      .arg(v.property("fileName").toString())
      .arg(v.property("lineNumber").toInt())
      .arg(v.property("message").toString())
      .arg(v.property("stack").toString());

    return Variant(allocError(ep::Result::ScriptException, qt::epFromQString(errorStr)));
  }
  else if (v.isObject())
  {
    Variant::VarMap::MapType varMap;
    QJSValueIterator i(v);
    while (i.hasNext())
    {
      i.next();

      QString name = i.name();
      QJSValue value = i.value();
      varMap.insert(ep::KeyValuePair(Variant(qt::epFromQString(name)), epToVariant(value)));
    }
    return std::move(varMap);
  }
  else if (v.isDate())
  {
  }
  else if (v.isRegExp())
  {
  }
  else if (v.isUndefined())
  {
    return Variant(Variant::Type::Void);
  }

  epDebugPrintf("epToVariant: Unsupported type!\n");
  return Variant();
}

void epFromVariant(const Variant &variant, QJSValue *pJSValue)
{
  switch (variant.type())
  {
    case Variant::Type::Void:
      *pJSValue = QJSValue(QJSValue::UndefinedValue);
      break;

    case Variant::Type::Null:
      *pJSValue = QJSValue(QJSValue::NullValue);
      break;

    case Variant::Type::Bool:
      *pJSValue = QJSValue(variant.asBool());
      break;

    case Variant::Type::Int:
    {
      int64_t i = variant.asInt();
      if (i >= 0)
        *pJSValue = QJSValue((uint32_t)i);
      else
        *pJSValue = QJSValue((int)i);
      break;
    }

    case Variant::Type::Float:
      *pJSValue = QJSValue(variant.asFloat());
      break;

    //case Variant::Type::Component:

    case Variant::Type::String:
      *pJSValue = QJSValue(variant.asString().toStringz());
      break;

    case Variant::Type::Array:
    {
      using namespace qt;

      uint length = (uint)variant.arrayLen();
      QJSValue val = QtApplication::kernel()->qmlEngine()->newArray(length);
      Slice<Variant> varr = variant.asArray();
      for (uint i = 0; i < length; ++i)
      {
        QJSValue t;
        epFromVariant(varr[i], &t);
        val.setProperty(i, t);
      }
      *pJSValue = val;
      break;
    }

    case Variant::Type::SharedPtr:
      switch (variant.spType())
      {
        case Variant::SharedPtrType::Component:
        {
          ep::ComponentRef spComponent = variant.asComponent();
          QObject *pQObject = nullptr;
          QQmlEngine::ObjectOwnership ownership = QQmlEngine::JavaScriptOwnership;

          // if the variant contains a qt::QObjectComponent then unpack this and give the direct QObject*
          if (spComponent->isType("ep.QObjectComponent"))
          {
            pQObject = shared_pointer_cast<qt::QObjectComponent>(spComponent)->getQObject();

            // since QML will steal ownership, we need to restore the prev state (in case it was CppOwnership)
            // note that this is only important for SharedPtr<QObjectComponent>'s since we don't know who created
            // the object we're passing through.
            ownership = QQmlEngine::objectOwnership(pQObject);
          }
          else
          {
            pQObject = qt::BuildQtEPComponent::Create(std::move(spComponent));
          }

          *pJSValue = qt::QtApplication::kernel()->qmlEngine()->newQObject(pQObject);
          if (ownership != QQmlEngine::JavaScriptOwnership)
            QQmlEngine::setObjectOwnership(pQObject, ownership);

          break;
        }

        case Variant::SharedPtrType::AssocArray:
        {
          using namespace qt;

          *pJSValue = QtApplication::kernel()->qmlEngine()->newObject();
          Variant::VarMap aa = variant.asAssocArray();
          for (auto v : aa)
          {
            if (!v.key.is(Variant::Type::String) && !v.key.is(Variant::Type::Int))
            {
              epDebugPrintf("epFromVariant: Key is not string!\n");
              continue;
            }
            QJSValue value;
            QString key;
            epFromVariant(v.key, &key);
            epFromVariant(v.value, &value);
            pJSValue->setProperty(key, value);
          }
          break;
        }
        default:
          break;
      }
      break;

    default:
      epDebugPrintf("epFromVariant: Unsupported type '%d'\n", (int)variant.type());
  }
}

Variant epToVariant(const QVariantMap &varMap)
{
  Variant::VarMap::MapType v;
  QVariantMap::const_iterator i = varMap.begin();
  while (i != varMap.end())
  {
    v.insert(ep::KeyValuePair(Variant(qt::epFromQString(i.key())), epToVariant(i.value())));
    ++i;
  }
  return std::move(v);
}

#else
EPEMPTYFILE
#endif
