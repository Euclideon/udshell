#include "hal/driver.h"

#if UDUI_DRIVER == UDDRIVER_QT

#include "signaltodelegate.h"
#include "kernel.h"

extern ud::Kernel *s_pKernel;

namespace qt
{

QMetaMethod QtSignalToDelegate::lookupSignalHandler(const QMetaMethod &m)
{
  int methodIndex = -1;
  QByteArray methodSig = m.methodSignature();
  int indexOfOpenBracket = methodSig.indexOf('(');
  if (indexOfOpenBracket != -1)
  {
    udMutableString128 signalHandlerName;
    signalHandlerName.concat("SignalHandler", methodSig.right(methodSig.size() - indexOfOpenBracket).data());
    s_pKernel->LogDebug(3, udSharedString::concat("Checking for signal handler: ", signalHandlerName.toStringz()));
    methodIndex = metaObject()->indexOfMethod(signalHandlerName.toStringz());
  }

  // TODO: remove this before release, maybe allow fallback on most closest match and error log/warn?
  if (methodIndex == -1)
  {
    UDASSERT(false, "Attempted to connect to unsupported signal: '%s' ", m.methodSignature().data());
    return QMetaMethod();
  }

  return metaObject()->method(methodIndex);
}

} // namespace qt

#endif  // UDUI_DRIVER == UDDRIVER_QT
