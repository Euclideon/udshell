#pragma once
#ifndef UDSCENE_H
#define UDSCENE_H

#include "udComponent.h"
#include "udSceneGraph.h"
#include "udView.h"


class udScene : public udComponent
{
  friend class udView;
public:
  virtual udResult InputEvent(const udInputEvent &ev);
  virtual udResult Update(double timeDelta);

  virtual udResult Render(udView *pView);

  udResult SetRenderModels(struct udRenderModel models[], size_t numModels);
  udResult SetRenderOptions(const struct udRenderOptions &options);

  udRenderScene *GetRenderScene(); // TODO: return immutable renderable scene, rebuild if bDirty

  static const udComponentDesc descriptor;

protected:
  double timeStep;

  udNode *pRootNode;
  bool bDirty; // becomes dirty when scene changes

  udRenderScene *pRenderScene;

  udRenderOptions options;
  udRenderModel renderModels[16];
  size_t numRenderModels;

  udScene(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams);
  virtual ~udScene();

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udScene, pType, pKernel, uid, initParams);
  }
};

#endif // UDSCENE_H
