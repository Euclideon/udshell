#pragma once
#ifndef EPVIEW_H
#define EPVIEW_H

#include "udRender.h"

#include "ep/cpp/math.h"
#include "component.h"
#include "hal/input.h"

namespace kernel
{

SHARED_CLASS(RenderableView);

PROTOTYPE_COMPONENT(Scene);
PROTOTYPE_COMPONENT(Camera);
PROTOTYPE_COMPONENT(View);

class View : public Component
{
public:
  EP_COMPONENT(View);

  virtual bool InputEvent(const epInputEvent &ev);
  virtual epResult Resize(int width, int height);

  RenderableViewRef GetRenderableView();

  void SetScene(SceneRef spScene);
  void SetCamera(CameraRef spCamera);
  SceneRef GetScene() const { return spScene; }
  CameraRef GetCamera() const { return spCamera; }

  void GetDimensions(int *pWidth, int *pHeight) const;
  void GetRenderDimensions(int *pWidth, int *pHeight) const;
  float GetAspectRatio() const { return (float)displayWidth / (float)displayHeight; }

  void SetRenderOptions(const udRenderOptions &_options) { this->options = _options; }
  const udRenderOptions& GetRenderOptions() const { return options; }

  void Activate();
  void Deactivate();

  Event<> Dirty;
  Event<> FrameReady;

  // TODO: remove these!
  void RegisterResizeCallback(void (*pCallback)(ViewRef pView, int w, int h)) { pResizeCallback = pCallback; }
  void RegisterPreRenderCallback(void(*pCallback)(ViewRef pView, SceneRef pScene)) { pPreRenderCallback = pCallback; }
  void RegisterPostRenderCallback(void(*pCallback)(ViewRef pView, SceneRef pScene)) { pPostRenderCallback = pCallback; }

  // TODO: Hack Remove me
  void ForceDirty() { OnDirty(); }

protected:
  friend class Renderer;

  SceneRef spScene = nullptr;
  CameraRef spCamera = nullptr;

  RenderableViewRef spLatestFrame = nullptr;

  int displayWidth = 0, displayHeight = 0;
  int renderWidth = 0, renderHeight = 0;

  // --- possibly derived stuff? ---
  udRenderOptions options;

  // TODO: remove these!
  void(*pResizeCallback)(ViewRef, int, int) = nullptr;
  void(*pPreRenderCallback)(ViewRef, SceneRef) = nullptr;
  void(*pPostRenderCallback)(ViewRef, SceneRef) = nullptr;

  View(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    memset(&options, 0, sizeof(options));

    // TODO: Remove this when Unsubscribe can be called with some kind of subscription identifier
    updateFunc = Delegate<void(double)>(this, &View::Update);
  }

  ~View() { Deactivate(); }

  void SetLatestFrame(UniquePtr<RenderableView> spFrame);
  void Update(double timeStep);
  void OnDirty();

  // TODO: Remove this when Unsubscribe can be called with some kind of subscription identifier
  Delegate<void(double)> updateFunc;
};

} // namespace kernel

#endif // EPVIEW_H
