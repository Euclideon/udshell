#pragma once
#ifndef UDSCENE_H
#define UDSCENE_H

#include "udRender.h"

#include "component.h"
#include "nodes/node.h"

namespace ud
{

SHARED_CLASS(RenderScene);

PROTOTYPE_COMPONENT(View);
PROTOTYPE_COMPONENT(Scene);

class Scene : public Component
{
  friend class View;
public:
  UD_COMPONENT(Scene);

  virtual udResult InputEvent(const udInputEvent &ev);
  virtual udResult Update(double timeDelta);

  NodeRef GetRootNode() const { return rootNode; }

  RenderSceneRef GetRenderScene();

  udEvent<> Dirty;

  // TODO: HACK: fix this api!
  udResult SetRenderModels(struct udRenderModel models[], size_t numModels);
  const udRenderModel* GetRenderModels(size_t *pNumModels) const { if (pNumModels) { *pNumModels = numRenderModels; } return renderModels; }

  void ForceDirty() { bDirty = true; }

protected:
  double timeStep = 0.0;

  NodeRef rootNode = nullptr;

  RenderSceneRef spCache = nullptr;
  bool bDirty = true; // becomes dirty when scene changes

  udRenderModel renderModels[16];
  size_t numRenderModels = 0;

  Scene(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams);
  virtual ~Scene();
};

};

#endif // UDSCENE_H
