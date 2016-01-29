#pragma once
#ifndef TYPECONVERT_H
#define TYPECONVERT_H

#include "ep/c/platform.h"
#include "ep/cpp/delegate.h"

#include "driver/qt/epqt.h"

#include <QString>
#include <QVariant>
#include <QJSValue>

#include "ep/cpp/variant.h"


// Qt type conversion to/from UD

namespace qt {

MutableString<ep::internal::VariantSmallStringSize> AllocUDStringFromQString(const QString &string);

class QtDelegate : public QObject
{
  Q_OBJECT

public:
  QtDelegate(ep::Variant::VarDelegate d) : QObject(nullptr), d(d) {}
  Q_INVOKABLE QVariant call(QVariant arg0 = QVariant(), QVariant arg1 = QVariant(), QVariant arg2 = QVariant(),
    QVariant arg3 = QVariant(), QVariant arg4 = QVariant(), QVariant arg5 = QVariant(), QVariant arg6 = QVariant(),
    QVariant arg7 = QVariant(), QVariant arg8 = QVariant(), QVariant arg9 = QVariant()) const;

private:
  ep::Variant::VarDelegate d;
};

class JSValueDelegate : public ep::DelegateMemento
{
protected:
  template<typename T>
  friend struct ep::SharedPtr;

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

Variant epToVariant(const QVariantMap &varMap);


namespace ep {
namespace internal {

template<> struct StringifyProxy<QChar*>        { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QChar *pS = *(QChar**)pData;       return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<const QChar*>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QChar *pS = *(const QChar**)pData; return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<QString>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const epVarArg *pArgs) { const QString *pS = (QString*)pData;     return ::epStringify(buffer, format, WString((const char16_t*)pS->data(), pS->size()), pArgs); }            static const size_t intify = 0; };

} // namespace internal
} // namespace ep



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

  inline QVariant QtDelegate::call(QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9) const
  {
    Variant varArgs[10] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 };

    int numArgs = 0;
    for (auto &arg : varArgs)
    {
      if (!arg.isValid())
        break;
      ++numArgs;
    }

    return d(Slice<Variant>(varArgs, numArgs)).as<QVariant>();
  }
}


#endif  // TYPECONVERT_H
