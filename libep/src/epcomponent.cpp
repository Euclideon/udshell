#include "ep/cpp/component.h"

ptrdiff_t epStringify(Slice<char> buffer, String format, Component *pComponent, const epVarArg *pArgs)
{
  ptrdiff_t len = pComponent->GetUID().length + 1;
  if (!buffer.ptr)
    return len;
  if (buffer.length < (size_t)len)
    return -len;

  // HACK: this could be a lot nicer!
  MutableString64 uid; uid.concat("@", pComponent->GetUID());
  return epStringifyTemplate(buffer, format, uid, pArgs);
}
