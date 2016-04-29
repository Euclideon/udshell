#pragma once
#if !defined(_EP_ISHADER_HPP)
#define _EP_ISHADER_HPP

#include "ep/cpp/component/component.h"

namespace ep {

struct ShaderType;

class IShader
{
public:
  virtual SharedString GetCode() const = 0;
  virtual void SetCode(SharedString code) = 0;

  virtual void SetType(ShaderType type) = 0;
  virtual ShaderType GetType() const = 0;

  virtual SharedPtr<RefCounted> GetRenderShader() = 0;
};

} // namespace ep

#endif // _EP_ISHADER_HPP
