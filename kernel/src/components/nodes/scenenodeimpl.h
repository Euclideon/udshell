#pragma once
#ifndef EP_SCENENODEIMPL_H
#define EP_SCENENODEIMPL_H

#include "ep/cpp/platform.h"

#include "ep/cpp/math.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/component/node/scenenode.h"
#include "ep/cpp/internal/i/iscenenode.h"
#include "hal/input.h"

namespace ep {

SHARED_CLASS(SceneNode);
SHARED_CLASS(Scene);

class RenderScene;

class SceneNodeImpl : public BaseImpl<SceneNode, ISceneNode>
{
public:
  SceneNodeImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  SceneRef GetScene() const override final { return spScene; }
  void SetScene(SceneRef _spScene) override final { spScene = _spScene; }

  Variant Save() const override final { return pInstance->Super::Save(); }

protected:
  void Render(RenderScene &spRenderScene, const Double4x4 &mat) override final;

  SceneRef spScene = nullptr;
};

} // namespace ep

#endif // EP_SCENENODEIMPL_H
