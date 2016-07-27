#pragma once
#ifndef EP_UDNODE_H
#define EP_UDNODE_H

#include "ep/cpp/platform.h"
#include "ep/cpp/math.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/iudnode.h"

struct udOctree;

namespace ep {

SHARED_CLASS(UDNode);
SHARED_CLASS(UDModel);

class RenderScene;

class UDNode : public Node
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, UDNode, IUDNode, Node, EPKERNEL_PLUGINVERSION, "UD model node", 0)

public:
  UDModelRef GetUDModel() const { return pImpl->GetUDModel(); }
  void SetUDModel(UDModelRef spModel) { pImpl->SetUDModel(spModel); }

  Variant save() const override { return pImpl->Save(); }

protected:
  UDNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  void render(RenderScene &spScene, const Double4x4 &mat) override { pImpl->Render(spScene, mat); }

private:
  Array<const PropertyInfo> getProperties() const;
};

} // namespace ep

#endif // EP_UDNODE_H
