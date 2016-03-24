#pragma once
#ifndef _GEOMNODE_H
#define _GEOMNODE_H

#include "ep/cpp/component/node/node.h"

namespace ep
{

SHARED_CLASS(Model);

SHARED_CLASS(GeomNode);
SHARED_CLASS(RenderScene);

class GeomNode : public Node
{
  EP_DECLARE_COMPONENT(GeomNode, Node, EPKERNEL_PLUGINVERSION, "GeomNode desc...", 0)
public:

  ModelRef GetModel() const { return spModel; }
  void SetModel(ModelRef _spModel) { spModel = _spModel; }

protected:
  GeomNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams) {}
  virtual ~GeomNode() {}

  void Render(RenderScene &spScene, const Double4x4 &mat) override;

  ModelRef spModel = nullptr;

  Array<const PropertyInfo> GetProperties() const;
};

} // namespace ep

#endif // _GEOMNODE_H
