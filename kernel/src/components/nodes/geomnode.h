#pragma once
#ifndef _GEOMNODE_H
#define _GEOMNODE_H

#include "node.h"

namespace ep
{

SHARED_CLASS(Model);

SHARED_CLASS(GeomNode);

class GeomNode : public Node
{
  EP_DECLARE_COMPONENT(GeomNode, Node, EPKERNEL_PLUGINVERSION, "GeomNode desc...")
public:

  ModelRef GetModel() const { return spModel; }
  void SetModel(ModelRef _spModel) { spModel = _spModel; }

protected:
  GeomNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams) {}
  virtual ~GeomNode() {}

  epResult Render(RenderSceneRef &spScene, const Double4x4 &mat) override;

  ModelRef spModel = nullptr;
};

} // namespace ep

#endif // _GEOMNODE_H
