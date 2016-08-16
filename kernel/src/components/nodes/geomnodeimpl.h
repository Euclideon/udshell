#pragma once
#ifndef EPGEOMNODEIMPL_H
#define EPGEOMNODEIMPL_H

#include "ep/cpp/component/node/geomnode.h"

namespace ep {

class GeomNodeImpl : public BaseImpl<GeomNode, IGeomNode>
{
public:
  GeomNodeImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  ModelRef GetModel() const override final { return spModel; }
  void SetModel(ModelRef _spModel) override final { spModel = _spModel; }

protected:
  void Render(RenderScene &spScene, const Double4x4 &mat) override final;

  ModelRef spModel = nullptr;
};

} // namespace ep

#endif // EPGEOMNODEIMPL_H
