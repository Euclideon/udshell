#pragma once
#ifndef EP_UDNODE_H
#define EP_UDNODE_H

#include "ep/cpp/math.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/iudnode.h"

struct udOctree;

namespace ep {

SHARED_CLASS(UDNode);
SHARED_CLASS(UDModel);

class RenderScene;

class UDNode : public Node, public IUDNode
{
  EP_DECLARE_COMPONENT_WITH_IMPL(UDNode, IUDNode, Node, EPKERNEL_PLUGINVERSION, "UD model node")

public:
  UDModelRef GetUDModel() const override { return pImpl->GetUDModel(); }
  void SetUDModel(UDModelRef spModel) override { pImpl->SetUDModel(spModel); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  UDNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  void Render(RenderScene &spScene, const Double4x4 &mat) override { pImpl->Render(spScene, mat); }

  static Array<const PropertyInfo> GetProperties()
  {
    return{
      EP_MAKE_PROPERTY(UDModel, "UDModel instance", nullptr, 0),
    };
  }
};

} // namespace ep

#endif // EP_UDNODE_H
