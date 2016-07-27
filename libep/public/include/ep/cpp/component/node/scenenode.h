#pragma once
#ifndef EP_SCENENODE_H
#define EP_SCENENODE_H

#include "ep/cpp/component/node/node.h"
#include "ep/cpp/internal/i/iscenenode.h"

namespace ep {

SHARED_CLASS(SceneNode);
SHARED_CLASS(Scene);

class RenderScene;

class SceneNode : public Node
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, SceneNode, ISceneNode, Node, EPKERNEL_PLUGINVERSION, "Scene node", 0)

public:
  SceneRef getScene() const { return pImpl->GetScene(); };
  void setScene(SceneRef spScene) { pImpl->SetScene(spScene); }

  Variant save() const override { return pImpl->Save(); }

protected:
  SceneNode(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Node(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  void render(RenderScene &spScene, const Double4x4 &mat) override { pImpl->Render(spScene, mat); }

private:
  Array<const PropertyInfo> getProperties() const;
};

} // namespace ep

#endif // EP_SCENENODE_H
