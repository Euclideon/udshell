#pragma once
#ifndef TYPECONVERT_H
#define TYPECONVERT_H

#include "ep/c/platform.h"

// Disabled Warnings
#if defined(EP_COMPILER_VISUALC)
# pragma warning(disable:4127) // conditional expression is constant
#endif //defined(_MSC_VER)

#include <QString>
#include <QVariant>
#include <QMetaType>
#include <QJSValue>

#include "ep/cpp/variant.h"


// Qt type conversion to/from UD

namespace qt {
String AllocUDStringFromQString(const QString &string);
}

inline Variant epToVariant(const QString &string)
{
  return Variant(qt::AllocUDStringFromQString(string), true);
}
void epFromVariant(const Variant &variant, QString *pString);

Variant epToVariant(const QVariant &var);
void epFromVariant(const Variant &variant, QVariant *pVariant);

inline Variant epToVariant(const QJSValue &jsValue)
{
  return epToVariant(jsValue.toVariant());
}
void epFromVariant(const Variant &variant, QJSValue *pJSValue);


namespace ep {
namespace internal {

template<> struct StringifyProxy<QChar*>        { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QChar *pS = *(QChar**)pData;       return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<const QChar*>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QChar *pS = *(const QChar**)pData; return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<QString>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QString *pS = (QString*)pData;     return ::epStringify(buffer, format, WString((const char16_t*)pS->data(), pS->size()), pArgs); }            static const size_t intify = 0; };

} // namespace internal
} // namespace ep

#endif  // TYPECONVERT_H
