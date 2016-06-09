#pragma once
#if !defined(_EP_IGEOMNODE_HPP)
#define _EP_IGEOMNODE_HPP

#include "ep/cpp/component/component.h"
#include "ep/cpp/input.h"

namespace ep
{

SHARED_CLASS(Model);
SHARED_CLASS(GeomNode);
SHARED_CLASS(RenderScene);

class IGeomNode
{
public:
  virtual ModelRef GetModel() const = 0;
  virtual void SetModel(ModelRef _spModel) = 0;

protected:
  virtual void Render(RenderScene &spScene, const Double4x4 &mat) = 0;
};

} // namespace ep

#endif // _EP_IGEOMNODE_HPP
