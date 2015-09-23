#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "../util/typeconvert.h"

#include "component_qt.h"

namespace qt
{

QVariant QtUDComponent::Get(const QString &name) const
{
  QByteArray byteArray = name.toUtf8();
  udString prop(byteArray.data(), byteArray.size());

  QVariant res;
  udFromVariant(spComponent->GetProperty(prop), &res);
  return res;
}

void QtUDComponent::Set(const QString &name, QVariant val)
{
  QByteArray byteArray = name.toUtf8();
  udString prop(byteArray.data(), byteArray.size());
  spComponent->SetProperty(prop, udToVariant(val));
}

QVariant QtUDComponent::Call(const QString &name) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udSlice<udVariant>()), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1), udToVariant(arg2)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1), udToVariant(arg2), udToVariant(arg3)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1),
    udToVariant(arg2), udToVariant(arg3), udToVariant(arg4)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFixedSlice<udVariant, 6> vargs = {  };
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1),
    udToVariant(arg2), udToVariant(arg3), udToVariant(arg4), udToVariant(arg5)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1),
    udToVariant(arg2), udToVariant(arg3), udToVariant(arg4), udToVariant(arg5), udToVariant(arg6)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1),
    udToVariant(arg2), udToVariant(arg3), udToVariant(arg4), udToVariant(arg5), udToVariant(arg6), udToVariant(arg7)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1),
    udToVariant(arg2), udToVariant(arg3), udToVariant(arg4), udToVariant(arg5), udToVariant(arg6),
    udToVariant(arg7), udToVariant(arg8)), &res);
  return res;
}

QVariant QtUDComponent::Call(const QString &name, QVariant arg0, QVariant arg1, QVariant arg2, QVariant arg3,
  QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, udToVariant(arg0), udToVariant(arg1),
    udToVariant(arg2), udToVariant(arg3), udToVariant(arg4), udToVariant(arg5),
    udToVariant(arg6), udToVariant(arg7), udToVariant(arg8), udToVariant(arg9)), &res);
  return res;
}

// Shim class
class JSValueDelegate : public udDelegateMemento
{
protected:
  template<typename T>
  friend class ::udSharedPtr;

  udVariant call(udSlice<udVariant> args)
  {
    QJSValueList jsArgs;
    jsArgs.reserve(static_cast<int>(args.length));

    for (auto &arg : args)
      jsArgs.append(arg.as<QJSValue>());

    QJSValue ret = jsVal.call(jsArgs);
    return udToVariant(ret);
  }

  JSValueDelegate(const QJSValue &jsValue) : jsVal(jsValue)
  {
    // set the memento to our call shim
    FastDelegate<udVariant(udSlice<udVariant>)> shim(this, &JSValueDelegate::call);
    m = shim.GetMemento();
  }

  ~JSValueDelegate() {}

  QJSValue jsVal;
};


void QtUDComponent::Subscribe(QString eventName, QJSValue func) const
{
  typedef udSharedPtr<JSValueDelegate> JSValueDelegateRef;

  if (!func.isCallable())
  {
    spComponent->LogError("Must subscribe to a javascript function. '{0}' is not callable.", func.toString().data());
    return;
  }

  QByteArray byteArray = eventName.toUtf8();
  udString event(byteArray.data(), byteArray.size());

  spComponent->Subscribe(event, udVariant::VarDelegate(JSValueDelegateRef::create(func)));
}

} // namespace qt

#endif
