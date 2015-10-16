#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "../epkernel_qt.h"
#include "signaltodelegate_qt.h"

namespace qt
{

QMetaMethod QtSignalToDelegate::lookupSignalHandler(const QMetaMethod &m)
{
  int methodIndex = -1;
  QByteArray methodSig = m.methodSignature();
  int indexOfOpenBracket = methodSig.indexOf('(');
  if (indexOfOpenBracket != -1)
  {
    MutableString128 signalHandlerName;
    signalHandlerName.concat("SignalHandler", methodSig.right(methodSig.size() - indexOfOpenBracket).data());
    QtApplication::Kernel()->LogDebug(3, SharedString::concat("Checking for signal handler: ", signalHandlerName));
    methodIndex = metaObject()->indexOfMethod(signalHandlerName.toStringz());
  }

  // TODO: remove this before release, maybe allow fallback on most closest match and error log/warn?
  if (methodIndex == -1)
  {
    EPASSERT(false, "Attempted to connect to unsupported signal: '{0}' ", m.methodSignature().data());
    return QMetaMethod();
  }

  return metaObject()->method(methodIndex);
}

} // namespace qt

#else
EPEMPTYFILE
#endif  // EPUI_DRIVER == EPDRIVER_QT
