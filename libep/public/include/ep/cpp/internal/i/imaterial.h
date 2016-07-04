#pragma once
#if !defined(_EP_IMATERIAL_HPP)
#define _EP_IMATERIAL_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/map.h"

namespace ep {

SHARED_CLASS(ArrayBuffer);
SHARED_CLASS(Shader);
SHARED_CLASS(Resource);

struct ShaderProperty;

struct ShaderType;
struct BlendMode;
struct CullMode;
struct StencilState;
struct CompareFunc;
struct TriangleFace;

class IMaterial
{
public:
  virtual const PropertyDesc *GetPropertyDesc(String _name, EnumerateFlags enumerateFlags = 0) const = 0;

  virtual Variant GetMaterialProperty(String property) const = 0;
  virtual void SetMaterialProperty(String property, Variant data) = 0;

  virtual ResourceRef Clone() const = 0;

#if 0 // When ranges are implemented
  const ShaderPropertyRange& GetShaderUniformsRange() const = 0;
  const ShaderPropertyRange& GetShaderAttributesRange() const = 0;
#endif
};

} // namespace ep

#endif // _EP_IMATERIAL_HPP
