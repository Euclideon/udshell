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

ep::MutableString<ep::internal::VariantSmallStringSize> AllocUDStringFromQString(const QString &string);

class QtDelegate : public QObject
{
  Q_OBJECT

public:
  QtDelegate(ep::VarDelegate d) : QObject(nullptr), d(d) {}
  Q_INVOKABLE QVariant call(QVariant arg0 = QVariant(), QVariant arg1 = QVariant(), QVariant arg2 = QVariant(),
    QVariant arg3 = QVariant(), QVariant arg4 = QVariant(), QVariant arg5 = QVariant(), QVariant arg6 = QVariant(),
    QVariant arg7 = QVariant(), QVariant arg8 = QVariant(), QVariant arg9 = QVariant()) const;

private:
  ep::VarDelegate d;
};

class JSValueDelegate : public ep::DelegateMemento
{
protected:
  template<typename T>
  friend struct ep::SharedPtr;

  ep::Variant call(ep::Slice<const ep::Variant> args);
  JSValueDelegate(const QJSValue &jsValue);
  ~JSValueDelegate() {}
  // TODO this needs to be pinned!!!
  QJSValue jsVal;
};

using JSValueDelegateRef = ep::SharedPtr<JSValueDelegate>;
}

inline ep::Variant epToVariant(const QString &string)
{
  return ep::Variant(qt::AllocUDStringFromQString(string));
}
void epFromVariant(const ep::Variant &variant, QString *pString);

ep::Variant epToVariant(const QVariant &var);
void epFromVariant(const ep::Variant &variant, QVariant *pVariant);

ep::Variant epToVariant(const QJSValue &jsValue);
void epFromVariant(const ep::Variant &variant, QJSValue *pJSValue);

ep::Variant epToVariant(const QVariantMap &varMap);


namespace ep {
namespace internal {

template<> struct StringifyProxy<QChar*>        { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const QChar *pS = *(QChar**)pData;       return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<const QChar*>  { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const QChar *pS = *(const QChar**)pData; return ::epStringify(buffer, format, WString((const char16_t*)pS, epStrlen((const char16_t*)pS)), pArgs); } static const size_t intify = 0; };
template<> struct StringifyProxy<QString>       { inline static ptrdiff_t stringify(Slice<char> buffer, String format, const void *pData, const VarArg *pArgs) { const QString *pS = (QString*)pData;     return ::epStringify(buffer, format, WString((const char16_t*)pS->data(), pS->size()), pArgs); }            static const size_t intify = 0; };

} // namespace internal
} // namespace ep



namespace qt {

  inline ep::Variant JSValueDelegate::call(ep::Slice<const ep::Variant> args)
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
    ep::VarDelegate::FastDelegateType shim(this, &JSValueDelegate::call);
    m = shim.GetMemento();
  }

  inline QVariant QtDelegate::call(QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9) const
  {
    ep::Variant varArgs[10] = { arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 };

    int numArgs = 0;
    for (auto &arg : varArgs)
    {
      if (!arg.isValid())
        break;
      ++numArgs;
    }

    return d(ep::Slice<ep::Variant>(varArgs, numArgs)).as<QVariant>();
  }
}


#endif  // TYPECONVERT_H
