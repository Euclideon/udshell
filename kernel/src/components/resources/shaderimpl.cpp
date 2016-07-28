#include "shaderimpl.h"
#include "renderresource.h"
#include "kernelimpl.h"

namespace ep {

  Array<const PropertyInfo> Shader::getProperties() const
  {
    return{
      EP_MAKE_PROPERTY("code", getCode, setCode, "The code string to be compiled", nullptr, 0),
      EP_MAKE_PROPERTY("type", getType, setType, "The type of the shader", nullptr, 0),
    };
  }

SharedPtr<RefCounted> ShaderImpl::GetRenderShader()
{
  if (!spCachedShader)
    spCachedShader = SharedPtr<RenderShader>::create(getKernel()->getImpl()->GetRenderer().ptr(), code, (epShaderType)type.v);

  return spCachedShader;
}

} // namespace ep
