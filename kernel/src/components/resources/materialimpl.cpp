#include "ep/cpp/component/component.h"
#include "ep/cpp/component/resource/text.h"
#include "materialimpl.h"
#include "shaderimpl.h"
#include "renderresource.h"
#include "kernelimpl.h"

namespace ep {

AVLTree<String, int> MaterialImpl::builtinProperties{
  { "vertexShader", 0 },
  { "pixelShader", 1 },
  { "geometryShader", 2 },
  { "tesselationControlShader", 3 },
  { "tesselationEvaluationShader", 4 },
  { "computeShader", 5 },
  { "blendMode", 6 },
  { "cullMode", 7 },
  { "depthFunc", 8 }
};


Variant epToVariant(const ShaderProperty &p)
{
  Variant::VarMap::MapType map;
  map.insert("type", p.type);
  map.insert("data", p.data);
  map.insert("current", p.current);
  map.insert("dirty", p.dirty);
  return std::move(map);
}

void epFromVariant(const Variant &variant, ShaderProperty *p)
{
  p->type = variant["type"].asSharedString();
  p->data = variant["data"];
  p->current = variant["current"].asBool();
  p->dirty = variant["dirty"].asBool();
}


Array<const PropertyInfo> Material::getProperties() const
{
  return{
    EP_MAKE_PROPERTY("vertexShader", getVertexShader, setVertexShader_Meta, "Vertex shader for rendering", nullptr, 0),
    EP_MAKE_PROPERTY("pixelShader", getPixelShader, setPixelShader_Meta, "Pixel shader for rendering", nullptr, 0),
    EP_MAKE_PROPERTY("geometryShader", getGeometryShader, setGeometryShader_Meta, "Geometry shader for rendering", nullptr, 0),
    EP_MAKE_PROPERTY("tesselationControlShader", getTesselationControlShader, setTesselationControlShader_Meta, "Tesselation Control shader for rendering", nullptr, 0),
    EP_MAKE_PROPERTY("tesselationEvaluationShader", getTesselationEvaluationShader, setTesselationEvaluationShader_Meta, "Tesselation Evaluation shader for rendering", nullptr, 0),

    EP_MAKE_PROPERTY("blendMode", getBlendMode, setBlendMode, "Frame buffer blend mode", nullptr, 0),
    EP_MAKE_PROPERTY("cullMode", getCullMode, setCullMode, "Back face cull mode", nullptr, 0),
    EP_MAKE_PROPERTY("depthCompareFunc", getDepthCompareFunc, setDepthCompareFunc, "Depth compare function", nullptr, 0),
  };
}

Array<const MethodInfo> Material::getMethods() const
{
  return{
    EP_MAKE_METHOD(getMaterialProperty, "Get a material property"),
    EP_MAKE_METHOD(setMaterialProperty, "Set a material property")
//      EP_MAKE_METHOD(getTexture, "Gets the texture ArrayBuffer at the given index"),
//      EP_MAKE_METHOD(setTexture, "Sets the texture ArrayBuffer at the given index"),
  };
}

void MaterialImpl::SetShader(ShaderType type, ShaderRef spShader)
{
  if (spShader)
    EPASSERT_THROW(spShader->getType() == type, Result::InvalidArgument, "Incorrect shader type!");

  if (shaders[type] == spShader)
    return;

  if (shaders[type])
    shaders[type]->changed.unsubscribe(Delegate<void()>(this, &MaterialImpl::OnShaderChanged));

  shaders[type] = spShader;

  if (spShader)
    spShader->changed.subscribe(Delegate<void()>(this, &MaterialImpl::OnShaderChanged));

  OnShaderChanged();
}

void MaterialImpl::OnShaderChanged()
{
  // remove shader properties from component
  // TODO...

  try
  {
    if (spShaderProgram)
      InvalidateProperties();

    BuildShaderProgram();
    PopulateShaderProperties();
  }
  catch (...)
  {
    spShaderProgram = nullptr;
  }

  // recreate the shader and populate properties
//  GetRenderProgram();
}

void MaterialImpl::BuildShaderProgram()
{
  // TODO: Add shader combination validation
  EPTHROW_IF(!(shaders[0] && shaders[1]), Result::Failure, "Missing shader program");

  Array<RenderShaderRef, NumShaders-1> renderShaders;
  for (ShaderRef &s : Slice<ShaderRef>(shaders, NumShaders-1))
  {
    if (s)
      renderShaders.pushBack(shared_pointer_cast<RenderShader>(s->getRenderShader()));
  }

  spShaderProgram = SharedPtr<RenderShaderProgram>::create(getKernel()->getImpl()->GetRenderer().get(), renderShaders);
}

void MaterialImpl::PopulateShaderProperties()
{
  EPASSERT_THROW(spShaderProgram, Result::Failure, "Shader program does not exist");

  for (size_t i = 0; i < spShaderProgram->numUniforms(); ++i)
  {
    uniforms.insert(spShaderProgram->getUniformName(i),
                    ShaderProperty
                    {
                      spShaderProgram->getUniformTypeString(i),
                      spShaderProgram->getUniform(i),
                      { spShaderProgram->getUniformType(i) },
                      i,
                      true,
                      false
                    }
                   );
  }

  for (size_t i = 0; i < spShaderProgram->numAttributes(); ++i)
  {
    attributes.insert(spShaderProgram->getAttributeName(i),
                    ShaderProperty
                    {
                      spShaderProgram->getAttributeTypeString(i),
                      Variant(), // ignored
                      { spShaderProgram->getAttributeType(i) } ,
                      true,
                      false // ignored
                    }
                  );
  }
}

const PropertyDesc *MaterialImpl::GetPropertyDesc(String _name, EnumerateFlags enumerateFlags) const
{
  return &dynamicPropertyCache.tryInsert(_name, [&]() {
    return PropertyDesc(
      { _name, _name, nullptr, 0 },
      MethodShim(&MaterialImpl::Get, SharedPtr<DynamicPropertyData>::create(_name)),
      MethodShim(&MaterialImpl::Set, SharedPtr<DynamicPropertyData>::create(_name))
    );
  });
}

Variant MaterialImpl::GetMaterialProperty(String property) const
{
  // look up from map
  int *pBuiltin = builtinProperties.get(property);
  if (pBuiltin)
  {
    switch (*pBuiltin)
    {
      // HAX: assume these indices map to the shader id's
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
        return GetShader(*pBuiltin);
      case 6: return GetBlendMode(); break;
      case 7: return GetCullMode(); break;
      case 8: return GetDepthCompareFunc(); break;
      default:
        EPASSERT(false, "Unreachable?!");
    }
  }

  // fall back to property map
  return uniforms[property].data;
}

void MaterialImpl::SetMaterialProperty(String property, Variant data)
{
  // look up from map
  int *pBuiltin = builtinProperties.get(property);
  if (pBuiltin)
  {
    switch (*pBuiltin)
    {
      // HAX: assume these indices map to the shader id's
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      {
        ShaderRef spShader;
        if (data.is(Variant::SharedPtrType::Component))
        {
          ComponentRef spC = data.asComponent();
          if (spC->getType().eq("ep.Shader"))
          {
            // data is a shader object
            spShader = shared_pointer_cast<Shader>(spC);
          }
          else if (spC->getType().eq("ep.Text"))
          {
            // data is a text buffer object (presumably containing shader source)
            spShader = getKernel()->createComponent<Shader>();
            spShader->setType(*pBuiltin);

            TextRef spText = shared_pointer_cast<Text>(spC);
            String s = spText->mapForRead();
            spShader->setCode(s);
            spText->unmap();
          }
        }
        else if (data.is(Variant::Type::String))
        {
          // data is a string buffer
          spShader = getKernel()->createComponent<Shader>();
          spShader->setType(*pBuiltin);
          spShader->setCode(data.asString());
        }
        EPASSERT_THROW(spShader, Result::InvalidArgument, "Expected shader object or shader text");

        SetShader(ShaderType(*pBuiltin), spShader);
        break;
      }
      case 6: SetBlendMode(data.as<BlendMode>()); break;
      case 7: SetCullMode(data.as<CullMode>()); break;
      case 8: SetDepthCompareFunc(data.as<CompareFunc>()); break;
      default:
        EPASSERT(false, "Unreachable?!");
    }
    return;
  }

  // fall back to property map
  uniforms[property].data = data;
  uniforms[property].dirty = true;
}

ResourceRef MaterialImpl::Clone() const
{
  MaterialRef spNewMat = getKernel()->createComponent<Material>();
  MaterialImpl* pImpl = spNewMat->getImpl<MaterialImpl>();

  for (size_t i = 0; i < NumShaders; i++)
    pImpl->shaders[i] = shaders[i];

  pImpl->textures = textures;
  pImpl->blendMode = blendMode;
  pImpl->cullMode = cullMode;
  pImpl->depthCompareFunc = depthCompareFunc;
  pImpl->spShaderProgram = spShaderProgram;

  pImpl->uniforms = uniforms;
  pImpl->attributes = attributes;
  pImpl->dynamicPropertyCache = dynamicPropertyCache;

  return spNewMat;
}

void MaterialImpl::InvalidateProperties()
{
  for (auto p : uniforms)
    p.value.current = false;

  for (auto a : attributes)
    a.value.current = false;
}

} // namespace ep
