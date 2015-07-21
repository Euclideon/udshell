#pragma once
#ifndef UDVIEW_H
#define UDVIEW_H

#include "udComponent.h"
#include "udInput.h"
#include "udRender.h"
#include "udMath.h"

struct udRenderableView;

PROTOTYPE_COMPONENT(udScene);
PROTOTYPE_COMPONENT(udCamera);
PROTOTYPE_COMPONENT(udView);

class udView : public udComponent
{
public:
  UD_COMPONENT(udView);

  virtual udResult InputEvent(const udInputEvent &ev);
  virtual udResult Resize(int width, int height);

  udResult Render(); // TODO: REMOVE ME

  udSharedPtr<const udRenderableView> GetRenderableView();

  void SetScene(udSceneRef spScene);
  void SetCamera(udCameraRef spCamera);
  udSceneRef GetScene() const { return spScene; }
  udCameraRef GetCamera() const { return spCamera; }

  void GetDimensions(int *pWidth, int *pHeight) const;
  void GetRenderDimensions(int *pWidth, int *pHeight) const;
  float GetAspectRatio() const { return (float)displayWidth / (float)displayHeight; }

  udEvent<> Dirty;

  // TODO: remove these!
  void RegisterResizeCallback(void (*pCallback)(udViewRef pView, int w, int h)) { pResizeCallback = pCallback; }
  void RegisterPreRenderCallback(void(*pCallback)(udViewRef pView, udSceneRef pScene)) { pPreRenderCallback = pCallback; }
  void RegisterPostRenderCallback(void(*pCallback)(udViewRef pView, udSceneRef pScene)) { pPostRenderCallback = pCallback; }

protected:
  udSceneRef spScene;
  udCameraRef spCamera;

  udSharedPtr<const udRenderableView> spCache;
  bool bDirty = true;

  int displayWidth = 0, displayHeight = 0;
  int renderWidth = 0, renderHeight = 0;

  // --- possibly derived stuff? ---
  udRenderOptions options;

  // TODO: remove these!
  void(*pResizeCallback)(udViewRef, int, int) = nullptr;
  void(*pPreRenderCallback)(udViewRef, udSceneRef) = nullptr;
  void(*pPostRenderCallback)(udViewRef, udSceneRef) = nullptr;

  udView(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
    : udComponent(pType, pKernel, uid, initParams) {}

  static udComponent *Create(const udComponentDesc *pType, udKernel *pKernel, udRCString uid, udInitParams initParams)
  {
    return udNew(udView, pType, pKernel, uid, initParams);
  }

  void OnDirty();
};

#endif // UDVIEW_H
