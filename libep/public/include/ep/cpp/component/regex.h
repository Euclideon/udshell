#pragma once
#ifndef EPREGEX_H
#define EPREGEX_H

#include "ep/cpp/internal/i/iregex.h"

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(Regex);

class Regex : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Regex, IRegex, Component, EPKERNEL_PLUGINVERSION, "Regular expression component via PCRE", 0)
public:

  virtual void CompilePattern(String pattern) { pImpl->CompilePattern(pattern); };
  virtual Array<String> Match(String text) { return pImpl->Match(text); };

protected:
  Regex(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }
};

}

#endif // EPREGEX_H
