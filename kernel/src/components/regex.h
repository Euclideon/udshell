#pragma once
#if !defined(_EP_REGEX_H)
#define _EP_REGEX_H

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(Regex);

class Regex : public Component
{
  EP_DECLARE_COMPONENT(ep, Regex, Component, EPKERNEL_PLUGINVERSION, "Regular expression component via PCRE", 0)
public:

  void CompilePattern(String pattern);
  Array<String> Match(String text);

protected:
  Regex(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  ~Regex();

  void *pCode = nullptr;
  void *pExtra = nullptr;
};

}

#endif // _EP_REGEX_H
