#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "typeconvert_qt.h"

#include "../components/component_qt.h"
#include "../components/qtcomponent_qt.h"
#include "../epkernel_qt.h"

#include <qqmlengine.h>
#include <QJSValueIterator>


// Qt type conversion to/from UD

namespace qt {

MutableString<ep::internal::VariantSmallStringSize> AllocUDStringFromQString(const QString &string)
{
  QByteArray byteArray = string.toUtf8();
  return MutableString<ep::internal::VariantSmallStringSize>(byteArray.data(), byteArray.size());
}

}

void epFromVariant(const Variant &variant, QString *pString)
{
  String s = variant.asString();
  if (!s.empty())
    *pString = QString::fromUtf8(s.ptr, static_cast<int>(s.length));
}

Variant epToVariant(QObject *pQObj)
{
  qt::QtEPComponent *pQC = qobject_cast<qt::QtEPComponent*>(pQObj);
  if (pQC)
    return Variant(pQC->GetComponent());

  udDebugPrintf("TODO: epToVariant: Unsupported QObject conversion '%s'; we need a global pKernel pointer >_<", pQObj->metaObject()->className());

  // TODO: create generic QtComponent which thinly wraps a QObject
//  pKernel->CreateComponent<QtComponent>({ { "object" }, { (int64_t)(size_t)pQObj } });
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
      return Variant(qt::AllocUDStringFromQString(var.toString()));

    case QMetaType::QObjectStar:
      return epToVariant(var.value<QObject*>());

    case QMetaType::User:
    {
      int userType = var.userType();
      if (userType == qMetaTypeId<QJSValue>())
        return epToVariant(var.value<QJSValue>());
    }
    default:
      udDebugPrintf("epToVariant: Unsupported type '%s' (support me!)\n", var.typeName());
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

    case Variant::Type::Component:
    {
      ep::ComponentRef spComponent = variant.asComponent();
      if (spComponent->IsType("qtcomponent"))
        pVariant->setValue(shared_pointer_cast<qt::QtComponent>(spComponent)->GetQObject());
      else
      {
        QObject *pQObject = new qt::QtEPComponent(spComponent);
        pVariant->setValue(pQObject);

        // since we allocated this QObject, we'll set it to QML ownership so it can be freed
        QQmlEngine::setObjectOwnership(pQObject, QQmlEngine::JavaScriptOwnership);
      }
      break;
    }

    case Variant::Type::Delegate:
    {
      EPASSERT(false, "TODO: Support passing delegates...");
      break;
    }

    // TODO: optimise?
    case Variant::Type::String:
      pVariant->setValue(variant.as<QString>());
      break;

    case Variant::Type::Array:
    {
      QVariantList list;
      Slice<Variant> arr = variant.asArray();
      list.reserve((int)arr.length);
      for (auto &v : arr)
      {
        QVariant t;
        epFromVariant(v, &t);
        list.push_back(t);
      }
      // TODO: what?! no move assignment! look into this...
      pVariant->setValue(list);
      break;
    }

    case Variant::Type::AssocArray:
    {
      QVariantMap map;
      Slice<KeyValuePair> aa = variant.asAssocArray();
      for (auto &v : aa)
      {
        if (!v.key.is(Variant::Type::String))
        {
          udDebugPrintf("epFromVariant: Key is not string!\n");
          continue;
        }
        String k = v.key.asString();
        QVariant value;
        epFromVariant(v.value, &value);
        map.insert(QLatin1String(k.ptr, (int)k.length), value);
      }
      // TODO: what?! no move assignment! look into this...
      pVariant->setValue(map);
      break;
    }

    default:
      udDebugPrintf("epFromVariant: Unsupported type '%d'\n", variant.type());
  }
}

Variant epToVariant(const QJSValue &v)
{
  // TODO: investigate if we can switch on some numeric type id?

  if (v.isNumber())
    return Variant(v.toNumber());
  else if (v.isString())
    return Variant(qt::AllocUDStringFromQString(v.toString()));
  else if (v.isBool())
    return Variant(v.toBool());
  else if (v.isNull())
    return Variant(nullptr);
  else if (v.isVariant())
    return epToVariant(v.toVariant());
  else if (v.isQObject())
    return epToVariant(v.toQObject());
  else if (v.isCallable())
    return Variant::VarDelegate(qt::JSValueDelegateRef::create(v));
  else if (v.isArray())
  {
    size_t length = (size_t)v.property(QString("length")).toNumber();
    Array<Variant> r(Reserve, length);

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
  else if (v.isObject())
  {
    Array<KeyValuePair> r;
    QJSValueIterator i(v);
    while (i.hasNext())
    {
      i.next();

      QString name = i.name();
      QJSValue value = i.value();
      r.pushBack(KeyValuePair(Variant(qt::AllocUDStringFromQString(name)), epToVariant(value)));
    }
    return Variant(std::move(r));
  }
  else if (v.isDate())
  {
  }
  else if (v.isError())
  {
  }
  else if (v.isRegExp())
  {
  }
  else if (v.isUndefined())
  {
    return Variant(Variant::Type::Void);
  }

  udDebugPrintf("epToVariant: Unsupported type!\n");
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

    case Variant::Type::Array:
    {
      using namespace qt;

      uint length = (uint)variant.arrayLen();
      QJSValue val = ((QtKernel*)QtApplication::Kernel())->QmlEngine()->newArray(length);
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
    //case Variant::Type::AssocArray:

    default:
      udDebugPrintf("epFromVariant: Unsupported type '%d'\n", variant.type());
  }
}

#endif
