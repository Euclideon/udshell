#pragma once
#ifndef TYPECONVERT_H
#define TYPECONVERT_H

#include <QString>
#include <QVariant>
#include <QMetaType>

#include "../components/component_qt.h"

#include "ep/epvariant.h"


// Qt type conversion to/from UD

inline epString AllocUDStringFromQString(const QString &string)
{
  QByteArray byteArray = string.toUtf8();

  // Need to do a deep copy and give ownership to the variant
  size_t length = byteArray.size() + 1;
  char *pString = udAllocType(char, length, udAF_None);
  memcpy(pString, byteArray.data(), length);

  return epString(pString, length - 1);
}

inline epVariant epToVariant(const QString &string)
{
  return epVariant(AllocUDStringFromQString(string), true);
}

inline void epFromVariant(const epVariant &variant, QString *pString)
{
  epString s = variant.asString();
  if (!s.empty())
    *pString = QString::fromUtf8(s.ptr, static_cast<int>(s.length));
}

inline epVariant epToVariant(const QVariant &var)
{
  if (!var.isValid() || var.isNull())
    return epVariant();

  switch (static_cast<QMetaType::Type>(var.type()))
  {
    // epVariant::Type::Null
    case QMetaType::Void:
      return epVariant();

    // epVariant::Type::Bool
    case QMetaType::Bool:
      return epVariant(var.toBool());

    // epVariant::Type::Float
    case QMetaType::Float:
    case QMetaType::Double:
      return epVariant(var.toDouble());

    // epVariant::Type::Int
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
      return epVariant((int64_t)var.toLongLong());

    // epVariant::Type::String
    // TODO: optimise - reduce unnecessary copies
    case QMetaType::QByteArray:
    case QMetaType::QString:
    case QMetaType::QChar:
      return epVariant(AllocUDStringFromQString(var.toString()), true);

    case QMetaType::QObjectStar:
    {
      QObject *pQObj = var.value<QObject*>();

      qt::QtEPComponent *pQC = qobject_cast<qt::QtEPComponent*>(pQObj);
      if (pQC)
        return epVariant(pQC->GetComponent());

      udDebugPrintf("epToVariant: Unsupported QObject conversion '%s'", pQObj->metaObject()->className());

      // TODO: create generic QtComponent which thinly wraps a QObject
//      pKernel->CreateComponent<QtComponent>({ { "object" }, { (int64_t)(size_t)pQObj } });
      return epVariant();
    }

    // TODO: serialize other types?

    // epVariant::Type::Array

    // epVariant::Type::AssocArray

    default:
      udDebugPrintf("epToVariant: Unsupported type '%s' (support me!)\n", var.typeName());
      return epVariant();
  };
}

inline void epFromVariant(const epVariant &variant, QVariant *pVariant)
{
  EPASSERT(pVariant->isNull(), "pVariant is not null");

  switch (variant.type())
  {
    case epVariant::Type::Null:
      // *pVariant is already null right?
      break;

    case epVariant::Type::Bool:
      pVariant->setValue(variant.asBool());
      break;

    case epVariant::Type::Int:
      pVariant->setValue(variant.asInt());
      break;

    case epVariant::Type::Float:
      pVariant->setValue(variant.asFloat());
      break;

    case epVariant::Type::Component:
      pVariant->setValue(qt::QtEPComponent(variant.asComponent()));
      break;

    //case epVariant::Type::Delegate:

    // TODO: optimise?
    case epVariant::Type::String:
      pVariant->setValue(variant.as<QString>());
      break;

    //case epVariant::Type::Array:

    //case epVariant::Type::AssocArray:

    default:
      udDebugPrintf("epFromVariant: Unsupported type '%d'\n", variant.type());
  };
}

inline epVariant epToVariant(const QJSValue &jsValue)
{
  return epToVariant(jsValue.toVariant());
}

inline void epFromVariant(const epVariant &variant, QJSValue *pJSValue)
{
  switch (variant.type())
  {
    case epVariant::Type::Null:
      *pJSValue = QJSValue(QJSValue::NullValue);
      break;

    case epVariant::Type::Bool:
      *pJSValue = QJSValue(variant.asBool());
      break;

    case epVariant::Type::Int:
    {
      int64_t i = variant.asInt();
      if (i >= 0)
        *pJSValue = QJSValue((uint32_t)i);
      else
        *pJSValue = QJSValue((int)i);
      break;
    }

    case epVariant::Type::Float:
      *pJSValue = QJSValue(variant.asFloat());
      break;

    //case epVariant::Type::Component:

    case epVariant::Type::String:
      *pJSValue = QJSValue(variant.asString().toStringz());

    //case epVariant::Type::Array:
    //case epVariant::Type::AssocArray:

    default:
      udDebugPrintf("epFromVariant: Unsupported type '%d'\n", variant.type());
  };
}

#endif  // TYPECONVERT_H
