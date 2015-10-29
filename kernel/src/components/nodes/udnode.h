#pragma once
#ifndef EP_UDNODE_H
#define EP_UDNODE_H

#include "udMath.h"
#include "udRender.h"

#include "ep/cpp/math.h"
#include "../component.h"
#include "node.h"
#include "components/resources/udmodel.h"
#include "hal/input.h"

struct udOctree;

namespace ep {

PROTOTYPE_COMPONENT(UDNode);

class UDNode : public Node
{
public:
  EP_COMPONENT(UDNode);

  UDModelRef GetUDModel() const { return spModel; }
  void SetUDModel(UDModelRef _spModel) { spModel = _spModel; }

protected:
  UDNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Node(pType, pKernel, uid, initParams) { }

  epResult Render(RenderSceneRef &spScene, const Double4x4 &mat) override;

  UDModelRef spModel = nullptr;
};

} // namespace ep

#endif // EP_UDNODE_H
