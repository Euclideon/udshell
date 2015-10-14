#pragma once
#ifndef TYPECONVERT_H
#define TYPECONVERT_H

#include <QString>
#include <QVariant>
#include <QMetaType>

#include "../components/component_qt.h"

#include "ep/cpp/variant.h"


// Qt type conversion to/from UD

inline String AllocUDStringFromQString(const QString &string)
{
  QByteArray byteArray = string.toUtf8();

  // Need to do a deep copy and give ownership to the variant
  size_t length = byteArray.size() + 1;
  char *pString = epAllocType(char, length, epAF_None);
  memcpy(pString, byteArray.data(), length);

  return String(pString, length - 1);
}

inline Variant epToVariant(const QString &string)
{
  return Variant(AllocUDStringFromQString(string), true);
}

inline void epFromVariant(const Variant &variant, QString *pString)
{
  String s = variant.asString();
  if (!s.empty())
    *pString = QString::fromUtf8(s.ptr, static_cast<int>(s.length));
}

inline Variant epToVariant(const QVariant &var)
{
  if (!var.isValid() || var.isNull())
    return Variant();

  switch (static_cast<QMetaType::Type>(var.type()))
  {
    // Variant::Type::Null
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
      return Variant(AllocUDStringFromQString(var.toString()), true);

    case QMetaType::QObjectStar:
    {
      QObject *pQObj = var.value<QObject*>();

      qt::QtEPComponent *pQC = qobject_cast<qt::QtEPComponent*>(pQObj);
      if (pQC)
        return Variant(pQC->GetComponent());

      udDebugPrintf("epToVariant: Unsupported QObject conversion '%s'", pQObj->metaObject()->className());

      // TODO: create generic QtComponent which thinly wraps a QObject
//      pKernel->CreateComponent<QtComponent>({ { "object" }, { (int64_t)(size_t)pQObj } });
      return Variant();
    }

    // TODO: serialize other types?

    // Variant::Type::Array

    // Variant::Type::AssocArray

    default:
      udDebugPrintf("epToVariant: Unsupported type '%s' (support me!)\n", var.typeName());
      return Variant();
  };
}

inline void epFromVariant(const Variant &variant, QVariant *pVariant)
{
  EPASSERT(pVariant->isNull(), "pVariant is not null");

  switch (variant.type())
  {
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
      pVariant->setValue(qt::QtEPComponent(variant.asComponent()));
      break;

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

inline Variant epToVariant(const QJSValue &jsValue)
{
  return epToVariant(jsValue.toVariant());
}

inline void epFromVariant(const Variant &variant, QJSValue *pJSValue)
{
  switch (variant.type())
  {
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

namespace ep {
namespace internal {

template<> struct StringifyProxy<QChar*>        { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QChar *pS = *(QChar**)pData;       return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<const QChar*>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QChar *pS = *(const QChar**)pData; return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<QString>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QString *pS = (QString*)pData;     return ::epStringify(buffer, format, WString((const char16_t*)pS->data(), pS->size()), pArgs); }            static const size_t intify = 0; };

} // namespace internal
} // namespace ep

#endif  // TYPECONVERT_H
