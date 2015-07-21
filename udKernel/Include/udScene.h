#pragma once
#ifndef UDSCENE_H
#define UDSCENE_H

#include "udComponent.h"
#include "udNode.h"
#include "udRender.h"

struct udRenderScene;

PROTOTYPE_COMPONENT(udView);
PROTOTYPE_COMPONENT(udScene);

class udScene : public udComponent
{
  friend class udView;
public:
  UD_COMPONENT(udScene);

  virtual udResult InputEvent(const udInputEvent &ev);
  virtual udResult Update(double timeDelta);

  udNodeRef GetRootNode() const { return rootNode; }

  udSharedPtr<const udRenderScene> GetRenderScene();

  udEvent<> Dirty;

  // TODO: HACK: fix this api!
  udResult SetRenderModels(struct udRenderModel models[], size_t numModels);
  udResult SetRenderOptions(const struct udRenderOptions &options);
  const udRenderModel* GetRenderModels(size_t *pNumModels) const { if (pNumModels) { *pNumModels = numRenderModels; } return renderModels; }
  const udRenderOptions& GetRenderOptions() const { return options; }

protected:
  double timeStep = 0.0;

  udNodeRef rootNode;

  udSharedPtr<const udRenderScene> spCache;
  bool bDirty = true; // becomes dirty when scene changes

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
