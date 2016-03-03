#include "ep/cpp/component/component.h"
#include "materialimpl.h"
#include "shaderimpl.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"

namespace ep {

void MaterialImpl::SetShader(ShaderType type, ShaderRef spShader)
{
  if (shaders[(int)type] == spShader)
    return;
  if (shaders[(int)type])
    shaders[(int)type]->Changed.Unsubscribe(Delegate<void()>(this, &MaterialImpl::OnShaderChanged));
  shaders[(int)type] = spShader;
  if (spShader)
    spShader->Changed.Subscribe(Delegate<void()>(this, &MaterialImpl::OnShaderChanged));
  OnShaderChanged();
}

void MaterialImpl::OnShaderChanged()
{
  // remove shader properties from component
  // TODO...

  spCachedShaderProgram = nullptr;

  // recreate the shader and populate properties
//  GetRenderProgram();
}

} // namespace ep
