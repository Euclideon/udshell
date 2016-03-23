#include "ep/cpp/component/component.h"
#include "materialimpl.h"
#include "shaderimpl.h"
#include "renderresource.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const PropertyInfo> Material::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(VertexShader, "Vertex shader for rendering", nullptr, 0),
    EP_MAKE_PROPERTY(PixelShader, "Pixel shader for rendering", nullptr, 0),
    EP_MAKE_PROPERTY(BlendMode, "Frame buffer blend mode", nullptr, 0),
    EP_MAKE_PROPERTY(CullMode, "Back face cull mode", nullptr, 0),
  };
}
Array<const MethodInfo> Material::GetMethods() const
{
  return{
    EP_MAKE_METHOD(GetTexture, "Gets the texture ArrayBuffer at the given index"),
    EP_MAKE_METHOD(SetTexture, "Sets the texture ArrayBuffer at the given index"),
    EP_MAKE_METHOD(SetMaterialProperty, "Set a Material Property"),
  };
}

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
