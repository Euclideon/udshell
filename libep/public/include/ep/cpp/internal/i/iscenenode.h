#pragma once
#if !defined(_EP_ISCENENODE_HPP)
#define _EP_ISCENENODE_HPP

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(Scene);

class RenderScene;

class ISceneNode
{
public:
  virtual SceneRef GetScene() const = 0;
  virtual void SetScene(SceneRef spScene) = 0;

  virtual Variant Save() const = 0;

protected:
  virtual void Render(RenderScene &spScene, const Double4x4 &mat) = 0;
};

} // namespace ep

#endif // _EP_ISCENENODE_HPP
