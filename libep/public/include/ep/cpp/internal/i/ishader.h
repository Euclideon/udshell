#pragma once
#if !defined(_EP_ISHADER_HPP)
#define _EP_ISHADER_HPP

#include "ep/cpp/string.h"

namespace ep {

class IShader
{
public:
  virtual SharedString GetCode() const = 0;
  virtual void SetCode(SharedString code) = 0;
};

} // namespace ep

#endif // _EP_ISHADER_HPP
