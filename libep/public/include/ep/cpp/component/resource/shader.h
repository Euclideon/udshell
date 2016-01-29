#pragma once
#if !defined(_EP_SHADER_HPP)
#define _EP_SHADER_HPP

#include "ep/cpp/component/resource/resource.h"
#include "ep/cpp/internal/i/ishader.h"

namespace ep {

SHARED_CLASS(Shader);

EP_ENUM(ShaderType,
        VertexShader,
        PixelShader);

class Shader : public Resource, public IShader
{
  EP_DECLARE_COMPONENT_WITH_IMPL(Shader, IShader, Resource, EPKERNEL_PLUGINVERSION, "Shader desc...")
public:
  SharedString GetCode() const override final { return pImpl->GetCode(); }
  void SetCode(SharedString code) override final { pImpl->SetCode(code); }

  // TODO: eager compile function, which attempts to compile the shader and produce error messages promptly
//  void Compile();

protected:
  Shader(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Resource(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(Code, "The code string to be compiled", nullptr, 0),
    };
  }
};

} // namespace ep

#endif // _EP_SHADER_HPP
