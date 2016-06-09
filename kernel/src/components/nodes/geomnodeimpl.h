#pragma once
#ifndef EPGEOMNODEIMPL_H
#define EPGEOMNODEIMPL_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/component/node/geomnode.h"
#include "ep/cpp/internal/i/igeomnode.h"

namespace ep
{

SHARED_CLASS(Model);
SHARED_CLASS(GeomNode);
SHARED_CLASS(RenderScene);

class GeomNodeImpl : public BaseImpl<GeomNode, IGeomNode>
{
public:
  GeomNodeImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  virtual ModelRef GetModel() const override final { return spModel; }
  virtual void SetModel(ModelRef _spModel) override final { spModel = _spModel; }

protected:
  void Render(RenderScene &spScene, const Double4x4 &mat) override final;

  ModelRef spModel = nullptr;
};

} // namespace ep

#endif // EPGEOMNODEIMPL_H
