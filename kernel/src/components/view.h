#pragma once
#ifndef EPVIEW_H
#define EPVIEW_H

#include "udRender.h"

#include "ep/cpp/math.h"
#include "ep/cpp/component/component.h"
#include "hal/input.h"

namespace ep {

SHARED_CLASS(RenderableView);

SHARED_CLASS(Scene);
SHARED_CLASS(Camera);
SHARED_CLASS(View);

class View : public Component
{
  EP_DECLARE_COMPONENT(View, Component, EPKERNEL_PLUGINVERSION, "View desc...")
public:

  virtual bool InputEvent(const epInputEvent &ev); // TODO: Method for this function?
  virtual void Resize(int width, int height);

  RenderableViewRef GetRenderableView() const;

  void SetScene(SceneRef spScene);
  SceneRef GetScene() const { return spScene; }

  void SetCamera(CameraRef spCamera);
  CameraRef GetCamera() const { return spCamera; }

  void GetDimensions(int *pWidth, int *pHeight) const;
  void GetRenderDimensions(int *pWidth, int *pHeight) const;
  float GetAspectRatio() const { return (float)displayWidth / (float)displayHeight; }

  void SetRenderOptions(const udRenderOptions &_options) { this->options = _options; } // TODO: Methods for these functions?
  const udRenderOptions& GetRenderOptions() const { return options; }

  void SetEnablePicking(bool enable);
  bool GetEnablePicking() const { return pickingEnabled; }

  void GoToBookmark(String bookmarkName);

  struct PickResult
  {
    Double3 position;
    udNodeIndex nodeIndex;
  };
  using PickDelegate = Delegate<void(const SharedArray<PickResult>&)>;
  using ScreenPoint = Vector2<int>;
  void RequestPick(SharedArray<const ScreenPoint> points, const PickDelegate &del); // TODO: Method for this function?

  ScreenPoint GetMousePosition() const { return mousePosition; }

  void Activate();
  void Deactivate();

  Event<> Dirty;
  Event<> FrameReady;
  Event<bool> EnabledPickingChanged;
  Event<Double3> PickFound;
  Event<ScreenPoint> MousePositionChanged;

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

  bool pickingEnabled = false;
  Double3 pickedPoint = { 0, 0, 0 };

  struct PickHighlightData
  {
    const udRenderModel *highlightModel;
    udNodeIndex highlightIndex;
  };
  PickHighlightData pickHighlightData = { nullptr, ~0ULL };

  ScreenPoint mousePosition = { 0, 0 };
  struct PickRequest
  {
    SharedArray<const ScreenPoint> points;
    PickDelegate pickDelegate;
  };

  Array<PickRequest> pickRequests;

  // TODO: remove these!
  void(*pResizeCallback)(ViewRef, int, int) = nullptr;
  void(*pPreRenderCallback)(ViewRef, SceneRef) = nullptr;
  void(*pPostRenderCallback)(ViewRef, SceneRef) = nullptr;

  View(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    memset(&options, 0, sizeof(options));
  }

  ~View() { Deactivate(); }

  void SetLatestFrame(UniquePtr<RenderableView> spFrame);
  void Update(double timeStep);
  void OnDirty();

  static Array<const PropertyInfo> GetProperties();
  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(GoToBookmark, "Move the Camera to the specified Bookmark"),
      EP_MAKE_METHOD(Activate, "Activate the View, e.g. start rendering"),
      EP_MAKE_METHOD(Deactivate, "Deactivate the View, e.g. stop rendering"),
      EP_MAKE_METHOD(Resize, "Resize the View"),
    };
  }
  static Array<const EventInfo> GetEvents()
  {
    return{
      EP_MAKE_EVENT(Dirty, "View dirty event"),
      EP_MAKE_EVENT(FrameReady, "The next frame has finished rendering"),
      EP_MAKE_EVENT(EnabledPickingChanged, "Enable Picking changed"),
      EP_MAKE_EVENT(PickFound, "Pick found"),
      EP_MAKE_EVENT(MousePositionChanged, "Mouse Position changed")
    };
  }

private:
  Variant::VarMap GetDimensionsProperty() const;
  Variant::VarMap GetRenderDimensionsProperty() const;
};


inline void View::SetEnablePicking(bool enable)
{
  if (pickingEnabled != enable)
  {
    pickingEnabled = enable;
    EnabledPickingChanged.Signal(pickingEnabled);
  }
}

} // namespace ep

#endif // EPVIEW_H
