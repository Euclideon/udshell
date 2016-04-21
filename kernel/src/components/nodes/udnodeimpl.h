#pragma once
#ifndef EP_UDNODEIMPL_H
#define EP_UDNODEIMPL_H

#include "ep/cpp/platform.h"
#include "udRender.h"

#include "ep/cpp/math.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/udnode.h"
#include "ep/cpp/internal/i/iudnode.h"
#include "hal/input.h"

struct udOctree;

namespace ep {

SHARED_CLASS(UDNode);
SHARED_CLASS(UDModel);

class RenderScene;

class UDNodeImpl : public BaseImpl<UDNode, IUDNode>
{
public:
  UDNodeImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  UDModelRef GetUDModel() const override final { return spModel; }
  void SetUDModel(UDModelRef _spModel) override final { spModel = _spModel; }

  Variant Save() const override final { return pInstance->Super::Save(); }

protected:
  void Render(RenderScene &spScene, const Double4x4 &mat) override final;

  UDModelRef spModel = nullptr;
};

} // namespace ep

#endif // EP_UDNODE_H
