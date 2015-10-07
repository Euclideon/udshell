#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "../util/typeconvert_qt.h"

#include "component_qt.h"

namespace qt
{

QVariant QtEPComponent::Get(const QString &name) const
{
  QByteArray byteArray = name.toUtf8();
  epString prop(byteArray.data(), byteArray.size());

  QVariant res;
  epFromVariant(wpComponent->GetProperty(prop), &res);
  return res;
}

void QtEPComponent::Set(const QString &name, QVariant val)
{
  QByteArray byteArray = name.toUtf8();
  epString prop(byteArray.data(), byteArray.size());
  wpComponent->SetProperty(prop, epToVariant(val));
}

QVariant QtEPComponent::Call(const QString &name) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epSlice<epVariant>()), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1), epToVariant(arg2)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1), epToVariant(arg2), epToVariant(arg3)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1),
    epToVariant(arg2), epToVariant(arg3), epToVariant(arg4)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epArray<epVariant, 6> vargs = {  };
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1),
    epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1),
    epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5), epToVariant(arg6)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1),
    epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5), epToVariant(arg6), epToVariant(arg7)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1),
    epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5), epToVariant(arg6),
    epToVariant(arg7), epToVariant(arg8)), &res);
  return res;
}

QVariant QtEPComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9) const
{
  QByteArray byteArray = name.toUtf8();
  epString method(byteArray.data(), byteArray.size());
  QVariant res;
  epFromVariant(wpComponent->CallMethod(method, epToVariant(arg0), epToVariant(arg1),
    epToVariant(arg2), epToVariant(arg3), epToVariant(arg4), epToVariant(arg5),
    epToVariant(arg6), epToVariant(arg7), epToVariant(arg8), epToVariant(arg9)), &res);
  return res;
}

// Shim class
class JSValueDelegate : public epDelegateMemento
{
protected:
  template<typename T>
  friend class ::epSharedPtr;

  epVariant call(epSlice<epVariant> args)
  {
    QJSValueList jsArgs;
    jsArgs.reserve(static_cast<int>(args.length));

    for (auto &arg : args)
      jsArgs.append(arg.as<QJSValue>());

    QJSValue ret = jsVal.call(jsArgs);
    return epToVariant(ret);
  }

  JSValueDelegate(const QJSValue &jsValue) : jsVal(jsValue)
  {
    // set the memento to our call shim
    FastDelegate<epVariant(epSlice<epVariant>)> shim(this, &JSValueDelegate::call);
    m = shim.GetMemento();
  }

  ~JSValueDelegate() {}

  QJSValue jsVal;
};


void QtEPComponent::Subscribe(QString eventName, QJSValue func) const
{
  typedef epSharedPtr<JSValueDelegate> JSValueDelegateRef;

  if (!func.isCallable())
  {
    wpComponent->LogError("Must subscribe to a javascript function. '{0}' is not callable.", (char16_t*)func.toString().utf16());
    return;
  }

  QByteArray byteArray = eventName.toUtf8();
  epString event(byteArray.data(), byteArray.size());

  wpComponent->Subscribe(event, epVariant::VarDelegate(JSValueDelegateRef::create(func)));
}

} // namespace qt

#endif
