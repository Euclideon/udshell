#pragma once
#ifndef TYPECONVERT_H
#define TYPECONVERT_H

#include "ep/c/platform.h"
#include "ep/cpp/delegate.h"

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
MutableString<ep::internal::VariantSmallStringSize> AllocUDStringFromQString(const QString &string);

class JSValueDelegate : public ep::DelegateMemento
{
protected:
  template<typename T>
  friend class ep::SharedPtr;

  Variant call(Slice<Variant> args);
  JSValueDelegate(const QJSValue &jsValue);
  ~JSValueDelegate() {}
  // TODO this needs to be pinned!!!
  QJSValue jsVal;
};

using JSValueDelegateRef = SharedPtr<JSValueDelegate>;
}

inline Variant epToVariant(const QString &string)
{
  return Variant(qt::AllocUDStringFromQString(string));
}
void epFromVariant(const Variant &variant, QString *pString);

Variant epToVariant(const QVariant &var);
void epFromVariant(const Variant &variant, QVariant *pVariant);

Variant epToVariant(const QJSValue &jsValue);
void epFromVariant(const Variant &variant, QJSValue *pJSValue);


namespace ep {
namespace internal {

template<> struct StringifyProxy<QChar*>        { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QChar *pS = *(QChar**)pData;       return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<const QChar*>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QChar *pS = *(const QChar**)pData; return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<QString>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QString *pS = (QString*)pData;     return ::epStringify(buffer, format, WString((const char16_t*)pS->data(), pS->size()), pArgs); }            static const size_t intify = 0; };

} // namespace internal
} // namespace kernel



namespace qt {

  inline Variant JSValueDelegate::call(Slice<Variant> args)
  {
    QJSValueList jsArgs;
    jsArgs.reserve(static_cast<int>(args.length));

    for (auto &arg : args)
      jsArgs.append(arg.as<QJSValue>());

    QJSValue ret = jsVal.call(jsArgs);
    return epToVariant(ret);
  }

  inline JSValueDelegate::JSValueDelegate(const QJSValue &jsValue) : jsVal(jsValue)
  {
    // set the memento to our call shim
    FastDelegate<Variant(Slice<Variant>)> shim(this, &JSValueDelegate::call);
    m = shim.GetMemento();
  }
}


#endif  // TYPECONVERT_H
