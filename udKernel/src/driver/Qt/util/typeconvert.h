#pragma once
#ifndef TYPECONVERT_H
#define TYPECONVERT_H

#include <QString>
#include <QVariant>
#include <QMetaType>

#include "../components/component_qt.h"

#include "util/udvariant.h"

// Qt type conversion to/from UD

inline udString AllocUDStringFromQString(const QString &string)
{
  QByteArray byteArray = string.toUtf8();

  // Need to do a deep copy and give ownership to the variant
  size_t length = byteArray.size() + 1;
  char *pString = udAllocType(char, length, udAF_None);
  memcpy(pString, byteArray.data(), length);

  return udString(pString, length - 1);
}

inline udVariant udToVariant(const QString &string)
{
  return udVariant(AllocUDStringFromQString(string), true);
}

inline void udFromVariant(const udVariant &variant, QString *pString)
{
  udString s = variant.asString();
  if (!s.empty())
    *pString = QString::fromUtf8(s.ptr, static_cast<int>(s.length));
}

inline udVariant udToVariant(const QVariant &var)
{
  if (var.isNull())
    return udVariant();

  switch (static_cast<QMetaType::Type>(var.type()))
  {
    // udVariant::Type::Null
    case QMetaType::Void:
      return udVariant();

    // udVariant::Type::Bool
    case QMetaType::Bool:
      return udVariant(var.toBool());

    // udVariant::Type::Float
    case QMetaType::Float:
    case QMetaType::Double:
      return udVariant(var.toDouble());

    // udVariant::Type::Int
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
      return udVariant((int64_t)var.toLongLong());

    // udVariant::Type::String
    // TODO: optimise - reduce unnecessary copies
    case QMetaType::QByteArray:
    case QMetaType::QString:
    case QMetaType::QChar:
      return udVariant(AllocUDStringFromQString(var.toString()), true);

    case QMetaType::QObjectStar:
    {
      QObject *pQObj = var.value<QObject*>();

      qt::QUDComponent *pQC = qobject_cast<qt::QUDComponent*>(pQObj);
      if (pQC)
        return udVariant(pQC->GetComponent());

      udDebugPrintf("udToVariant: Unsupported QObject conversion '%s'", pQObj->metaObject()->className());

      // TODO: create generic QtComponent which thinly wraps a QObject
//      pKernel->CreateComponent<QtComponent>({ { "object" }, { (int64_t)(size_t)pQObj } });
      return udVariant();
    }

    // TODO: serialize other types?

    // udVariant::Type::Array

    // udVariant::Type::AssocArray

    default:
      udDebugPrintf("udToVariant: Unsupported type '%s' (support me!)\n", var.typeName());
      return udVariant();
  };
}

inline void udFromVariant(const udVariant &variant, QVariant *pVariant)
{
  UDASSERT(pVariant->isNull(), "pVariant is not null");

  switch (variant.type())
  {
    case udVariant::Type::Null:
      // *pVariant is already null right?
      break;

    case udVariant::Type::Bool:
      pVariant->setValue(variant.asBool());
      break;

    case udVariant::Type::Int:
      pVariant->setValue(variant.asInt());
      break;

    case udVariant::Type::Float:
      pVariant->setValue(variant.asFloat());
      break;

    case udVariant::Type::Component:
      pVariant->setValue(qt::QUDComponent(variant.asComponent()));
      break;

    //case udVariant::Type::Delegate:

    // TODO: optimise?
    case udVariant::Type::String:
      pVariant->setValue(variant.as<QString>());
      break;

    //case udVariant::Type::Array:

    //case udVariant::Type::AssocArray:

    default:
      udDebugPrintf("udToVariant: Unsupported type '%d'\n", variant.type());
  };
}

#endif  // TYPECONVERT_H
