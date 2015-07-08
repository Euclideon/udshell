#pragma once
#ifndef UDSCENE_H
#define UDSCENE_H

#include "udComponent.h"
#include "udSceneGraph.h"
#include "udRender.h"

PROTOTYPE_COMPONENT(udView);
PROTOTYPE_COMPONENT(udScene);


class udScene : public udComponent
{
  friend class udView;
public:
  UD_COMPONENT(udScene);

  virtual udResult InputEvent(const udInputEvent &ev);
  virtual udResult Update(double timeDelta);

  virtual udResult Render(udViewRef pView);

  // TODO: HACK: fix this api!
  udResult SetRenderModels(struct udRenderModel models[], size_t numModels);
  udResult SetRenderOptions(const struct udRenderOptions &options);
  const udRenderModel* GetRenderModels(size_t *pNumModels) const { if (pNumModels) { *pNumModels = numRenderModels; } return renderModels; }
  const udRenderOptions& GetRenderOptions() const { return options; }

  udRenderScene *GetRenderScene(); // TODO: return immutable renderable scene, rebuild if bDirty

protected:
  double timeStep = 0.0;

  udNode *pRootNode = nullptr;
  bool bDirty = false; // becomes dirty when scene changes

  udRenderScene *pRenderScene = nullptr;

  udRenderOptions options;
  udRenderModel renderModels[16];
  size_t numRenderModels = 0;

  udScene(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams);
  virtual ~udScene();

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udScene, pType, pKernel, uid, initParams);
  }
};

#endif // UDSCENE_H
