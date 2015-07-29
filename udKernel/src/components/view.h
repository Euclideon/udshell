#pragma once
#ifndef UDVIEW_H
#define UDVIEW_H

#include "udRender.h"
#include "udMath.h"

#include "component.h"
#include "hal/input.h"

namespace ud
{

SHARED_CLASS(RenderableView);

PROTOTYPE_COMPONENT(Scene);
PROTOTYPE_COMPONENT(Camera);
PROTOTYPE_COMPONENT(View);

class View : public Component
{
public:
  UD_COMPONENT(View);

  virtual udResult InputEvent(const udInputEvent &ev);
  virtual udResult Resize(int width, int height);

  udResult Render(); // TODO: REMOVE ME

  RenderableViewRef GetRenderableView();

  void SetScene(SceneRef spScene);
  void SetCamera(CameraRef spCamera);
  SceneRef GetScene() const { return spScene; }
  CameraRef GetCamera() const { return spCamera; }

  void GetDimensions(int *pWidth, int *pHeight) const;
  void GetRenderDimensions(int *pWidth, int *pHeight) const;
  float GetAspectRatio() const { return (float)displayWidth / (float)displayHeight; }

  void SetRenderOptions(const udRenderOptions &options) { this->options = options; }
  const udRenderOptions& GetRenderOptions() const { return options; }

  udEvent<> Dirty;

  // TODO: remove these!
  void RegisterResizeCallback(void (*pCallback)(ViewRef pView, int w, int h)) { pResizeCallback = pCallback; }
  void RegisterPreRenderCallback(void(*pCallback)(ViewRef pView, SceneRef pScene)) { pPreRenderCallback = pCallback; }
  void RegisterPostRenderCallback(void(*pCallback)(ViewRef pView, SceneRef pScene)) { pPostRenderCallback = pCallback; }

  // TODO: Hack Remove me
  void ForceDirty() { OnDirty(); }

protected:
  SceneRef spScene = nullptr;
  CameraRef spCamera = nullptr;

  RenderableViewRef spCache = nullptr;
  bool bDirty = true;

  int displayWidth = 0, displayHeight = 0;
  int renderWidth = 0, renderHeight = 0;

  // --- possibly derived stuff? ---
  udRenderOptions options;

  // TODO: remove these!
  void(*pResizeCallback)(ViewRef, int, int) = nullptr;
  void(*pPreRenderCallback)(ViewRef, SceneRef) = nullptr;
  void(*pPostRenderCallback)(ViewRef, SceneRef) = nullptr;

  View(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams) { memset(&options, 0, sizeof(options)); }

  static Component *Create(const ComponentDesc *pType, Kernel *pKernel, udRCString uid, InitParams initParams)
  {
    return udNew(View, pType, pKernel, uid, initParams);
  }

  void OnDirty();
};
} // namespace ud
#endif // UDVIEW_H
