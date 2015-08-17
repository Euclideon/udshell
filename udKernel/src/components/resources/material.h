#pragma once
#ifndef _UD_MATERIAL_H
#define _UD_MATERIAL_H

#include "components/resources/resource.h"
#include "util/udsharedptr.h"
#include "util/udstring.h"

namespace ud
{

SHARED_CLASS(Material);


enum class MaterialProperties
{
  __VectorProps = 0,
  DiffuseColor = __VectorProps,
  AmbientColor,
  EmissiveColor,
  SpecularColor,
  __MaxVectorProp,

  Max = __MaxVectorProp,
  NumVectorProps = __MaxVectorProp - __VectorProps
};


class Material : public Resource
{
public:
  UD_COMPONENT(Material);

  void SetMaterialProperty(MaterialProperties property, const udFloat4 &val)
  {
    vecProps[(size_t)property] = val;
  }

protected:
  Material(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, udInitParams initParams)
    : Resource(pType, pKernel, uid, initParams) {}
  virtual ~Material() {}

  udFloat4 vecProps[MaterialProperties::NumVectorProps];
};

} // namespace ud

#endif // _UD_MATERIAL_H
