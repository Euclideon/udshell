#pragma once
#ifndef EP_UDNODE_H
#define EP_UDNODE_H

#include "udMath.h"
#include "udRender.h"

#include "ep/cpp/math.h"
#include "ep/cpp/component/component.h"
#include "node.h"
#include "components/resources/udmodel.h"
#include "hal/input.h"

struct udOctree;

namespace ep {

SHARED_CLASS(UDNode);

class UDNode : public Node
{
  EP_DECLARE_COMPONENT(UDNode, Node, EPKERNEL_PLUGINVERSION, "UD model node")
public:

  UDModelRef GetUDModel() const { return spModel; }
  void SetUDModel(UDModelRef _spModel) { spModel = _spModel; }

protected:
  UDNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams) { }

  epResult Render(RenderSceneRef &spScene, const Double4x4 &mat) override;

  UDModelRef spModel = nullptr;

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(UDModel, "UDModel instance", nullptr, 0),
    };
  }
};

} // namespace ep

#endif // EP_UDNODE_H
