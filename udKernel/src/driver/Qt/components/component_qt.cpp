#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "../util/typeconvert.h"

#include "component_qt.h"

namespace qt
{

QVariant QtComponent::Get(QString name) const
{
  QByteArray byteArray = name.toUtf8();
  udString prop(byteArray.data(), byteArray.size());

  QVariant res;
  udFromVariant(spComponent->GetProperty(prop), &res);
  return res;
}
void QtComponent::Set(QString name, QVariant val)
{
  QByteArray byteArray = name.toUtf8();
  udString prop(byteArray.data(), byteArray.size());
  spComponent->SetProperty(prop, udToVariant(val));
}

QVariant QtComponent::Call(QString name, QVariantList args) const
{
  QByteArray byteArray = name.toUtf8();
  udString method(byteArray.data(), byteArray.size());

  udFixedSlice<udVariant, 10> vargs;
  vargs.reserve(args.size());
  for (int i = 0; i < args.size(); ++i)
    vargs[i] = udToVariant(args[i]);
  QVariant res;
  udFromVariant(spComponent->CallMethod(method, vargs), &res);
  return res;
}

void QtComponent::Subscribe(QString eventName, QJSValue func) const
{
  UDASSERT(func.isCallable(), "!");

  UDASSERT(false, "TODO");
}

} // namespace qt

#endif
