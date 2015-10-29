#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "typeconvert_qt.h"

#include "../components/component_qt.h"
#include "../components/qtcomponent_qt.h"

#include <qqmlengine.h>


// Qt type conversion to/from UD

namespace qt {

String AllocUDStringFromQString(const QString &string)
{
  QByteArray byteArray = string.toUtf8();

  // Need to do a deep copy and give ownership to the variant
  size_t length = byteArray.size() + 1;
  char *pString = epAllocType(char, length, epAF_None);
  memcpy(pString, byteArray.data(), length);

  return String(pString, length - 1);
}

}

void epFromVariant(const Variant &variant, QString *pString)
{
  String s = variant.asString();
  if (!s.empty())
    *pString = QString::fromUtf8(s.ptr, static_cast<int>(s.length));
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
      return Variant(qt::AllocUDStringFromQString(var.toString()), true);

    case QMetaType::QObjectStar:
    {
      QObject *pQObj = var.value<QObject*>();

      qt::QtEPComponent *pQC = qobject_cast<qt::QtEPComponent*>(pQObj);
      if (pQC)
        return Variant(pQC->GetComponent());

      udDebugPrintf("TODO: epToVariant: Unsupported QObject conversion '%s'; we need a global pKernel pointer >_<", pQObj->metaObject()->className());

      // TODO: create generic QtComponent which thinly wraps a QObject
//      pKernel->CreateComponent<QtComponent>({ { "object" }, { (int64_t)(size_t)pQObj } });
      return Variant(nullptr);
    }

    // TODO: serialize other types?

    // Variant::Type::Array

    // Variant::Type::AssocArray

    default:
      udDebugPrintf("epToVariant: Unsupported type '%s' (support me!)\n", var.typeName());
      return Variant();
  };
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

    //case Variant::Type::Delegate:

    // TODO: optimise?
    case Variant::Type::String:
      pVariant->setValue(variant.as<QString>());
      break;

    //case Variant::Type::Array:

    //case Variant::Type::AssocArray:

    default:
      udDebugPrintf("epFromVariant: Unsupported type '%d'\n", variant.type());
  };
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

    //case Variant::Type::Array:
    //case Variant::Type::AssocArray:

    default:
      udDebugPrintf("epFromVariant: Unsupported type '%d'\n", variant.type());
  };
}

#endif
