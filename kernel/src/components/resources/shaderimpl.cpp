#include "shaderimpl.h"
#include "renderresource.h"
#include "kernelimpl.h"

namespace ep {

  Array<const PropertyInfo> Shader::GetProperties() const
  {
    return{
      EP_MAKE_PROPERTY(Code, "The code string to be compiled", nullptr, 0),
      EP_MAKE_PROPERTY(Type, "The type of the shader", nullptr, 0),
    };
  }

SharedPtr<RefCounted> ShaderImpl::GetRenderShader()
{
  if (!spCachedShader)
    spCachedShader = SharedPtr<RenderShader>::create(GetKernel()->GetImpl()->GetRenderer().ptr(), code, (epShaderType)type.v);

  return spCachedShader;
}

} // namespace ep
