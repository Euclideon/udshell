#pragma once
#if !defined(_EP_IREGEX_HPP)
#define _EP_IREGEX_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(Regex);

class IRegex
{
public:
  virtual void CompilePattern(String pattern) = 0;
  virtual Array<String> Match(String text) = 0;
};

}

#endif // _EP_IREGEX_HPP
