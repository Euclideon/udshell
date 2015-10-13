#pragma once
#ifndef _GEOMNODE_H
#define _GEOMNODE_H

#include "node.h"

namespace ep
{

SHARED_CLASS(Model);

PROTOTYPE_COMPONENT(GeomNode);

class GeomNode : public Node
{
public:
  EP_COMPONENT(GeomNode);

  ModelRef GetModel() const { return spModel; }
  void SetModel(ModelRef spModel) { this->spModel = spModel; }

protected:
  GeomNode(const ComponentDesc *pType, Kernel *pKernel, epSharedString uid, epInitParams initParams)
    : Node(pType, pKernel, uid, initParams) {}
  virtual ~GeomNode() {}

  epResult Render(RenderSceneRef &spScene, const udDouble4x4 &mat) override;

  ModelRef spModel = nullptr;
};

} // namespace ep

#endif // _GEOMNODE_H
